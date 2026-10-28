# flex documentation

General information about flex is at <https://westes.github.io/flex/>.
The manual is at <https://westes.github.io/flex/manual/>, with the whole
manual on a single page at
<https://westes.github.io/flex/manual-single.html>.

What is in this directory:

* `flex.texi` - the source of the manual, in Texinfo.
* `flex.1` - the man page. It is generated from `flex --help` with
  help2man during the build.
* `site/` - the source of the landing page at
  <https://westes.github.io/flex/>.

`make info` builds the manual as Info files, and `make html` builds it as
HTML, one page per section, in `flex.html/`. A single-page HTML manual
comes from `makeinfo --html --no-split -I ../examples/manual flex.texi`.

The website is built from these files by the Website workflow in
`.github/workflows/website.yml`. It runs when a release tag is pushed, or
when run by hand, and replaces the `gh-pages` branch.
