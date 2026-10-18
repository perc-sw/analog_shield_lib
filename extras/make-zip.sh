#!/usr/bin/env bash
# Builds the Arduino library ZIP (for Sketch -> Include Library -> Add .ZIP Library...)
# from a committed git ref, using git archive. The export-ignore entries in
# .gitattributes leave out tests, docs and CI files; README.md, CHANGELOG.md
# and LICENSE are left out below.
#
#   extras/make-zip.sh [ref] [output-dir]
#     ref         commit, branch or tag to package (default HEAD)
#     output-dir  where to write <name>-<version>.zip (default build/)
#
# Name and version come from library.properties at that ref. Prints the ZIP path.
# Also used by .github/workflows/release.yml.
set -euo pipefail

ref="${1:-HEAD}"
out_dir="${2:-build}"

cd "$(git rev-parse --show-toplevel)"

props="$(git show "$ref:library.properties")"
name="$(sed -n 's/^name=//p' <<<"$props" | tr -d '\r')"
version="$(sed -n 's/^version=//p' <<<"$props" | tr -d '\r')"
if [[ -z "$name" || -z "$version" ]]; then
  echo "error: name= or version= missing from library.properties at $ref" >&2
  exit 1
fi

# Uncommitted changes are not in a git archive; say so rather than ship stale files
if [[ "$ref" == "HEAD" && -n "$(git status --porcelain --untracked-files=no)" ]]; then
  echo "warning: uncommitted changes are not included in the ZIP" >&2
fi

mkdir -p "$out_dir"
zip_path="$out_dir/$name-$version.zip"
# One top-level folder named after the library, as the Arduino IDE expects.
# The IDE does not use README.md, CHANGELOG.md or LICENSE, so they are left out
# here only; GitHub's "Source code" archives still include them.
git archive --format=zip --prefix="$name/" -o "$zip_path" "$ref" -- . \
  ':(exclude)README.md' ':(exclude)CHANGELOG.md' ':(exclude)LICENSE'
echo "$zip_path"
