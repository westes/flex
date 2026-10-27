# Build image for flex's CI: the tools the workflows used to apt-get on every
# run, GNU gettext 1.0, which Ubuntu 22.04 does not package, and the previous
# release of flex, which a git checkout needs in order to build.
#
# Build it from the top of the source tree, so tools/bootstrap-flex is in the
# build context:
#
#     docker build -f .github/ci-image/Dockerfile -t flex-ci .

# Pinned by digest. Dependabot's docker ecosystem can propose new digests.
FROM ubuntu:22.04@sha256:e0f80c829509ef4df9e76bd8944eb7b8ec25cbb379b112a7f9448b2c77a77c9b AS gettext

ARG GETTEXT_VERSION=1.0
ARG GETTEXT_SHA256=71132a3fb71e68245b8f2ac4e9e97137d3e5c02f415636eb508ae607bc01add7
# Bruno Haible, who signed the release.
ARG GETTEXT_SIGNER_FPR=E0FFBD975397F77A32AB76ECB6301D9E1BBEAC08

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
        build-essential ca-certificates curl gpgv xz-utils \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /src
RUN curl -fsSLO https://ftp.gnu.org/gnu/gettext/gettext-${GETTEXT_VERSION}.tar.xz \
 && curl -fsSLO https://ftp.gnu.org/gnu/gettext/gettext-${GETTEXT_VERSION}.tar.xz.sig \
 && curl -fsSLO https://ftp.gnu.org/gnu/gnu-keyring.gpg

# The hash pins the exact tarball. The signature check needs the signing key's
# own fingerprint on a VALIDSIG line, so a swapped keyring does not help.
RUN echo "${GETTEXT_SHA256}  gettext-${GETTEXT_VERSION}.tar.xz" | sha256sum -c - \
 && gpgv --keyring ./gnu-keyring.gpg --status-fd 1 \
        gettext-${GETTEXT_VERSION}.tar.xz.sig gettext-${GETTEXT_VERSION}.tar.xz \
    | grep -q "^\[GNUPG:\] VALIDSIG ${GETTEXT_SIGNER_FPR} "

# Bundled libraries and no optional extras, so the result needs only the C and
# C++ runtime libraries.
RUN tar -xf gettext-${GETTEXT_VERSION}.tar.xz \
 && mkdir build && cd build \
 && ../gettext-${GETTEXT_VERSION}/configure --prefix=/usr/local \
        --disable-java --disable-curses --disable-acl --disable-openmp \
        --with-included-libxml --with-included-libunistring --without-emacs \
 && make -j"$(nproc)" \
 && make install DESTDIR=/staging


FROM ubuntu:22.04@sha256:e0f80c829509ef4df9e76bd8944eb7b8ec25cbb379b112a7f9448b2c77a77c9b

# The same tools the workflows installed, minus gettext and autopoint. The
# hosted runner image preinstalls others that the workflows never listed,
# which a bare container lacks: build-essential, git and certificates are
# added here, and curl is for the bootstrap script below.
RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y \
        build-essential ca-certificates curl git \
        gcc autoconf automake libtool bison help2man lzip texinfo texlive \
 && rm -rf /var/lib/apt/lists/*

COPY --from=gettext /staging/usr/local /usr/local
RUN ldconfig

# A git checkout has no scan.c, and building it needs a flex that is already
# installed. Build the previous release, which ships a scan.c.
COPY tools/bootstrap-flex /tmp/bootstrap-flex
RUN sh /tmp/bootstrap-flex /usr/local && rm /tmp/bootstrap-flex
