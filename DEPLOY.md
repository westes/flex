# Automatic Deployment with Github Actions

The .github/workflows directory contains Github Actions scripts that build the master branch in response to three events:
1. When changes are pushed to Github
2. When a pull request is submitted
3. When a Milestone is closed

The build.yml script handles cases 1 and 2. The milestone.yml script handles case 3.
There is code duplication between these scripts. It's a side-effect of the Github Actions syntax and fixes are being explored.

The Milestone script takes a number of actions beyond a basic build & test:
* It updates the NEWS file to include the titles of closed issues assigned to the Milestone
* It update the flex version number in configure.ac to match the Milestone title
* It creates a new tag in the repository as the starting point for a Release
* It triggers a release build that produces signed tarballs from the new tag

## Setup for Code Signing
Code signing is based on GPG and Github repository secrets.

We'll discuss repository setup first, then key setup first.

## Github Repository Secrets
Go to your repository's settings and create two "Repository secrets" called:
- GPG_SIGNING_KEY
- GPG_SIGNING_PASSWD

The contents of these keys are an ASCII armored GPG key with the Sign capability and the passphrase to unlock that key.
If you already have keys you're comfortable using, paste them into the window when you set up the secrets. Otherwise, you
can update the value of a secret at any time by returning to the Settings>Secrets page and clicking Update.

## GPG Key Preparation
 Since the scripts are meant to run unattended, you may not want to use your regular GPG signing key. 
 This section lays out a pattern for creating a detached Certifying and Signing key pair, with different passphrases. 
 This will help you revoke your Signing key if your CI toolchain (e.g. Github Actions) is compromised.

### Create a temporary GNUPGHOME
Create a temporary gpg working directory while you build your key pair. This simplifies changing only the Signing key's
passphrase later on.

    > mygpghome=.tmpgpg
	> mkdir -m 700 $mygpghome
	> export GNUPGHOME=$mygpghome/
 
### Create a Certifying Key
Create a key with only the Certifying capability. This reserves the Sign, Encrypt, and Authenticate capabilities for subkeys.
The following example creates a primary Certifying key using 4096-bit RSA with a validity period of 3 years.
Fill in your name and email address, adjust the algorithm, and change the validity period as you see fit. The word "cert"
is required to set the key capabilities. A validity period of 1 - 3 years is recommended for this key.

    > gpg --quick-gen-key 'Full Name <email@address.com>' rsa4096 cert 30d

You'll be asked to under a passphrase. This will remain private so make it memorable and strong.

If everything works, you'll see output like the following that includes your Certifying key's fingerprint.

    Note that this key cannot be used for encryption.  You may want to use
    the command "--edit-key" to generate a subkey for this purpose.
    pub   rsa4096 2021-03-27 [C] [expires: 2021-04-26]
          902C9CD6E95F4429A45A56BE3ADF9765D4D7E1F8
    uid                      Full Name <email@address.com>

### Create a Signing Subkey
Create a subkey with only the Signing capability. The subkey uses the same algorithm as the primary key, but may have a shorter
validity period. A validity period of up to 1 year is recommended for this key. You'll use the primary key fingerprint as an
argument to this command.

    > gpg --quick-add-key CBB56298C56B9A8E98CDDDC87FEBE45739D9C223 rsa4096 sign 15d
	
You'll be asked for the passphrase of your primary key. If key generation is successfull you'll be returned to the command
promp without errors. Use the following command to check that your subkey was created.

    > gpg --list-secret-keys
	gpg: checking the trustdb
    gpg: marginals needed: 3  completes needed: 1  trust model: pgp
    gpg: depth: 0  valid:   1  signed:   0  trust: 0-, 0q, 0n, 0m, 0f, 1u
    gpg: next trustdb check due at 2021-04-26
    /home/user/.tmppgp/pubring.kbx
    -----------------------------
    sec   rsa4096 2021-03-27 [C] [expires: 2021-04-26]
          902C9CD6E95F4429A45A56BE3ADF9765D4D7E1F8
    uid           [ultimate] Full Name <email@address.com>
    ssb   rsa4096 2021-03-27 [S] [expires: 2021-04-11]
	
We'll need the fingerprint of the subkey in a moment. That's obtained by extending the previous command.

    > gpg --list-secret-keys --with-colons
	sec:u:4096:1:3ADF9765D4D7E1F8:1616804233:1619396233::u:::cSC:::+:::23::0:
    fpr:::::::::902C9CD6E95F4429A45A56BE3ADF9765D4D7E1F8:
    grp:::::::::AFD6763E4BE3060CE43266CA57B95AD49F672E53:
    uid:u::::1616804233::4870E7BDEC077F3D4C65C5770E1B3C08F14A88B7::Full Name <email@address.com>::::::::::0:
    ssb:u:4096:1:CFA1F2FA6F73BA07:1616804291:1618100291:::::s:::+:::23:
    fpr:::::::::5EC1D4EDD47DAB3C1CD91047CFA1F2FA6F73BA07:
    grp:::::::::AC82DE3133CE1A7DB065031E386D2141CACDDAC0:
	
The line that begins "ssb" is your Signing subkey and the "fpr" line below it contains your Signing key's fingerprint.

### Export your keys
We now have a Certifying & Signing key pair protected by a passphrase that you'll keep private. Export those keys
and keep them protected so you can generate new Signing keys, extend your Certifying key's validity, or revoke a 
key if one is ever compromised. We first export the public certificates, then the full key pair, and finally the
Signing key on its own.

    > gpg --export --armor --output public_cert.asc email@address.com
	> gpg --export-secret-keys --armor --output secrets.asc 902C9CD6E95F4429A45A56BE3ADF9765D4D7E1F8
	> gpg --export-secret-subkeys --armor --output sub_secrets.asc 5EC1D4EDD47DAB3C1CD91047CFA1F2FA6F73BA07!
	
Note that the public and primary secret key exports can take either your user ID or the key fingerprint as their argument.
The private subkey export takes your subkey fingerprint followed by an exclamation point.

Upload your public_cert.asc file to a PGP keyserver.
Back up all three of these files somewhere safe. For the remaining steps, we'll only need sub_secrets.asc.

### Reset GNUPGHOME
Now that your have exported your keys, we will delete the temporary GNUPGHOME and start a new one. This is the simplest way
to ensure that we're only changing the password of the Signing subkey.

    > mygpghome=.tmpgpg
	> rm -rf $mygpghome
	> mkdir -m  700 $mygpghome
	> export GNUPGHOME=$mygpghome/
	> gpg --list-secret-keys
    gpg: keybox '/home/user/.tmppgp/pubring.kbx' created
    gpg: /home/user/.tmppgp/trustdb.gpg: trustdb created
	
### Change the password on your Signing Subkey
The Certifying & Signing keys are stored together in the secrets.asc file. The Signing key is stored alone in sub_secrets.asc.
We can make a copy of the Signing key that uses its own passphrase by importing only from sub_secrets.asc.

    > gpg --import sub_secrets.asc
	[ provide your primary key's passphrase when prompted ]
	
	> gpg --list-secret-keys
	/home/user/.tmppgp/pubring.kbx
    -----------------------------
    sec#  rsa4096 2021-03-27 [C] [expires: 2021-04-26]
          902C9CD6E95F4429A45A56BE3ADF9765D4D7E1F8
    uid           [ unknown] Full Name <email@address.com>
    ssb   rsa4096 2021-03-27 [S] [expires: 2021-04-11]
	
We know the primary key is detached by the octothorpe (#) beside its name. Also, the trust level in the key is "unknown" rather
that "ultimate" as it was when the primary key was present.

Now we can change the password and export the Signing key again.

    > gpg --passwd email@address.com
	[ provide your primary key's passphrase a final time ]
	[ provide a new passphrase for the Signing key, twice ]
	
	> gpg --export-secret-subkeys --armor --output sign_secret.asc 5EC1D4EDD47DAB3C1CD91047CFA1F2FA6F73BA07!
	[ provide your Signing key's new passphrase when prompted ]
	
