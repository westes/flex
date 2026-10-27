#!/bin/sh
# shell for updating the translations before a release

# Let this be executed in the po/ subdir.
cd "$(dirname "$0")" || exit

echo "Updating translations via TP"
po-fetch TP flex || exit

# po-fetch lists only the languages the TP has, and deletes the generated
# English quoting variants. Both are rebuilt from the POT file, so put
# them back in LINGUAS.
{
    sed -n '/^#/p' LINGUAS
    { sed '/^#/d' LINGUAS; echo en@boldquot; echo en@quot; } | LC_ALL=C sort
} >LINGUAS.new && mv LINGUAS.new LINGUAS

echo "Regenerating POT file and recompiling PO files..."
make update-po
