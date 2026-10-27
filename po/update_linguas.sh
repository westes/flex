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

# A few Translation Project files lack the Language header field, which
# po-fetch warns about. Add it, taken from the file name.
for po in *.po; do
    lang=${po%.po}
    case $lang in en@*) continue ;; esac
    if ! grep -q '^"Language:' "$po"; then
        LC_ALL=C sed "/^\"Language-Team:/a\\
\"Language: $lang\\\\n\"" "$po" >"$po.new" && mv "$po.new" "$po"
    fi
done

echo "Regenerating POT file and recompiling PO files..."
make update-po
