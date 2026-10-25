#!/bin/sh
# Build Magic Pockets from your own copy of the game:
#     ./build.sh "Magic Pockets (1993) (Renegade).zip"
# then: cd recomp && ./pockets
# The steps are recipe.json's "build", run by SuperSpider's tools/weave.py.
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
SUPERSPIDER="${SUPERSPIDER:-$HOME/stuff/superspider}"
[ $# -gt 0 ] || { echo "usage: $0 \"Magic Pockets (1993) (Renegade).zip\"" >&2; exit 2; }
(cd "$SUPERSPIDER" && dune build)
WEAVE_RECIPES="$HERE" exec python3 -I "$SUPERSPIDER/tools/weave.py" build "$@"
