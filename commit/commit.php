#!/bin/env php
<?php

function usage($command)
{
    echo "Usage: $command <date>\n";
    echo "\n";
    echo "date: YYYY-MM-DD\n";
    exit(1);
}

if ($argc !== 2) {
    usage($argv[0]);
}

list($year, $month, $day) = explode('-', $argv[1]);
if (!checkdate($month, $day, $year)) {
    usage($argv[0]);
}

$since = "--since='$argv[1]T00:00:00'";
$until = "--until='$argv[1]T23:59:59'";
$commit = shell_exec("git log -1 --format=%ct $since $until");
if (is_numeric($commit)) {
    $date = date('Y-m-d H:i:s', $commit + mt_rand(10 * 60, 60 * 60));
} else {
    $date = date(
        'Y-m-d H:i:s',
        strtotime("$argv[1] 06:00:00") + mt_rand(10 * 60, 60 * 60)
    );
}

$authorDate = "GIT_AUTHOR_DATE='$date'";
$committerDate = "GIT_COMMITTER_DATE='$date'";

echo shell_exec("$authorDate $committerDate git commit");
