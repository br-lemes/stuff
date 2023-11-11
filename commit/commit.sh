#!/bin/sh

DATE="$(randtime $1)"
export GIT_AUTHOR_DATE="$DATE"
export GIT_COMMITTER_DATE="$DATE"

git commit
