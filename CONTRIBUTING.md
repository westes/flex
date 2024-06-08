# CONTRIBUTING to Flex

## Introduction

Thank you for your interest in contributing to Flex! Flex's [issue
tracker](https://github.com/westes/flex/issues) is on GitHub. That's
the best place to find a task that needs attention. It's also the best
place to check whether a problem you've found is known to the
community.

## Baseline Build Environment

The maintenance baseline build environment is Ubuntu 20.04 LTS.

Flex needs a previous version of itself in order to build a new
version of itself. You can either install an operating system package
of flex, or you can build flex from one of the release tar balls.

Configuring your Flex development environment to match the maintenance
baseline will help you collaborate and review contributions with the
other developers working on Flex. See the
[INSTALL](https://github.com/westes/flex/blob/master/INSTALL.md) for
programs required to build flex from scratch.

## Branching Convention

Flex source code is maintained in Git at GitHub. Each Flex developer
forks their own copy of the main repository at
[westes/flex](https://github.com/westes/flex). Flex development occurs
in feature branches of the forks. PRs will eventually be submitted
from the feature branches of the forks to westes/flex.

Flex development forks are not required to reside on GitHub. However:
- GitHub's pull request (PR) process only works with repositories
  hosted on GitHub;
- Flex's automated testing pipeline depends on GitHub's Actions
  infrastructure and may not function on other Git servers.

Therefore, if your fork of flex resides outside of github, you may
wish to submit your patches via email, using standard git mechanisms.

## Preparing Your Commits

Small PRs are easier to review and merge. Minimize the scope of the
changes in a single PR within reason. Changes that touch one or two
files are likely to be reviewed and accepted more quickly than changes
that touch every file in Flex. Your pull request should do one atomic
thing unless there is a really good reason for your pull request to do
more than one thing.

Format your commit messages following [Conventional Commits
1.0.0](https://www.conventionalcommits.org/en/v1.0.0/) (CC BY 3.0
netlify.com). Briefly:

```
    <type>[(optional scope)][!]: <description>
	[BLANK LINE]
	[optional body]
	[BLANK LINE]
	[optional footer(s)]
```

A typical commit message might look like:

```
    docs: Add CONTRIBUTING manual
	
	Refs: #1234
```

The description should summarize the changes in a single, short
sentence. The description should be written in the imperative mood,
like the descriptions of the types below.

The primary types are:
- build: Change the build system, including tracking external dependencies
- ci: Change the CI/CD system
- docs: Update the package documentation
- feat: Add a new feature
- fix: Fix a bug or incorrect behavior
- perf: Improve performance of a feature
- refactor: Simplify maintainability without affecting performance or behavior
- revert: Revert a previous commit
- style: Correct a style mismatch (indentation, etc.)
- test: Change or add to the test suite

The optional scope must appear in parentheses and must consist of a
noun describing the section of the code base that was changed.

```
    docs(contrib): Update URLs in CONTRIBUTING manual
```

The optional `!` must appear before the description whenever the
commit introduces a breaking change - one that breaks compatibility
with previous versions of the code base. Whenever the `!` marker
appears, a BREAKING-CHANGE footer should also be included.

The optional body may be included to provid additional information
or context about the change. If it appears, it must be preceded by a
blank line.

The optional footers may be included to provide additional referential
information and links to tracked issues, PRs, etc. Each footer must be
preceded by a blank line. Footers must be formatted as `<footer-type>:
<description>`, similar to the first line of commit messages. The
description's format depends upon the footer-type. Known footer-types
include:
- Refs: A comma-separated list of commit hashes and/or issue and PR numbers. Issue and PR numbers must be prefixed with a `#`.
- BREAKING-CHANGE: A description of the change that breaks compatibility with previous versions of the code, including the version number at which compatibility is broken. 

A breaking change commit message might look like:

```
    feat!: Switch to quantum scanner
    
    BREAKING-CHANGE: New scanner runs in constant time but doesn't support any existing hardware. 
	Breaks compatibility with 1.0.0.
```

Squash your commits so that each commit is an atomic change. This aids
discussion and revision during the review process. In particular,
there should be no commits that fixup prior commits.

## Submitting PRs

Before submitting a PR to flex, test your changes either locally or
using the GitHub Actions pipeline.

### Testing locally

1. Commit and/or stash your changes.
1. Clean your working copy using

```
git clean -xdf
```

1. Build flex following the directions in [INSTALL](https://www.github.com/westes/flex/blob/master/INSTALL.md).
1. Run both the `make check` and `make distcheck` targets.

### Testing with GitHub Actions

If you created a fork of Flex on GitHub, any PR you make to your own
main branch will trigger the build and test pipeline.

1. Commit your changes.
1. Push your local branch to your remote at GitHub. Assuming your GitHub remote is called origin:

```
git push origin feature_branch
```

To submit a pull request through GitHub's web interface:

1. Open your GitHub Flex repository in your web browser.
1. Click the 'Pull requests' link.
1. Click the 'New pull request' button.
1. Change the 'base repository' from 'westes/flex' to your fork.
1. Change the 'base' branch to 'master' if it isn't already set.
1. Change the 'compare' branch to your feature branch.
1. Click the 'Create pull request' button.
1. Click the 'Actions' link to monitor the progress of your build and test job.

### Submitting PRs to westes/flex

Sending a PR to westes/flex follows nearly the same process as sending one to your own main branch.

1. Commit your changes.
1. Push your local branch to your remote at GitHub. Assuming your GitHub remote is called origin:

```
git push origin feature_branch
```

To use GitHub's web interface:

1. Open your GitHub Flex repository in your web browser.
1. Click the 'Pull requests' link.
1. Click the 'New pull request' button.
1. Change the 'base repository' to 'westes/flex'.
1. Change the 'base' branch to 'master' if it isn't already set.
1. Change the 'compare' branch to your feature branch.
1. Click the 'Create pull request' button.
1. Add notes to your PR including 
    - A title or commit-like message
	- A summary of the commits in your pull request 
    - Issue numbers your PR covers 
	- Links to your GitHub Actions test results or a copy of the last few lines of output from your local test results.

If this is your first contribution to Flex, execution of the Actions
pipeline will have to be manually approved by the maintainer. If you
are a returning contributor, you can click the Actions link to watch
your job run.

Keep an eye on your PR's discussion page and your email for review
notes and questions from other developers.

Thanks for contributing!
