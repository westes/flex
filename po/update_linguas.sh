#!/bin/sh
# shell for updating the translations before a release

# Let this be executed in the po/ subdir.
cd "$(dirname "$0")" || exit

echo "Updating translations via TP"
rsync -Lrtvz  translationproject.org::tp/latest/flex/ . # || exit

# Are there now PO files that are not in svn yet?
NEWSTUFF=$(git status --porcelain *.po | grep "^??")

if [ -n "${NEWSTUFF}" ]; then
    echo "New languages found; updating LINGUAS"
    echo "# List of available languages." >LINGUAS
    echo $(printf '%s\n' *.po | LC_ALL=C sort | sed 's/\.po//g') >>LINGUAS
fi

echo "Regenerating POT file and remerging and recompiling PO files..."
make update-po

# Ensure that the PO files are newer than the POT.
touch *.po

# Compile PO files
make
