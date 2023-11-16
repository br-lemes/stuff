#!/usr/bin/env php
<?php

// cSpell: ignore breno, coloiado, difft, fazsite

$PATH = ['A' => '/srv/http/fazsite', 'B' => '/srv/http/coloiado'];

$cwd = getcwd();

if (substr_compare($PATH['A'], $cwd, 0, strlen($PATH['A'])) === 0) {
    $current = 'A';
    $target = 'B';
} elseif (substr_compare($PATH['B'], $cwd, 0, strlen($PATH['B'])) === 0) {
    $current = 'B';
    $target = 'A';
} else {
    echo "Not in $PATH[A] or $PATH[B]\n";
    exit(1);
}

if ($argc !== 2) {
    echo "Usage: $argv[0] <file>\n";
    exit(1);
}

$path = substr($cwd, strlen($PATH[$current]), strlen($cwd));
if (strlen($path) > 0) {
    $file = "$path/$argv[1]";
} else {
    $file = $argv[1];
}

system("difft --color always $PATH[$target]$file $PATH[$current]$file\n");
