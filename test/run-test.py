# Copyright (c) 2018 the Talvos developers. All rights reserved.
#
# This file is distributed under a three-clause BSD license. For full license
# terms please see the LICENSE file distributed with this source code.

import os
import subprocess
import sys

if len(sys.argv) < 3:
    print('Usage: python run-test.py [EXE...] EXE [ARGS...] TCF')
    exit(1)

filename = sys.argv[-1]

if not os.path.isfile(filename):
  print(f'TCF file not found: `{filename}`')
  sys.exit(1)

test_dir  = os.path.dirname(os.path.realpath(filename))
test_file = os.path.basename(filename)

os.chdir(test_dir)

# Run talvos-cmd
proc = subprocess.Popen(sys.argv[1:], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, stdin=sys.stdin)
(output, _) = proc.communicate()
retval = proc.returncode

output = output.decode('utf-8')
# print(output, file=sys.stderr)
print(output, file=sys.stdout)

oln = 0
outlines = output.splitlines()

# Loop over lines in test file
tcf = open(test_file)
lines = tcf.read().splitlines()
expected_exit_code = 0
expected_abort = False
rc = 0
for ln in range(len(lines)):
    line = lines[ln]

    if line.startswith('# CHECK: '):
        pattern = line[9:]
        matched = False
        for o in range(oln, len(outlines)):
            if pattern in outlines[o]:
                matched = True
                oln = o+1
                break
        if not matched:
            print('CHECK on line %d not found' % (ln+1))
            rc = 1
    elif line.startswith('# EXIT '):
        expected_exit_code = int(line[7:])
    elif line.startswith('# ABORT'):
        expected_abort = True
if expected_abort and retval == 0:
    print('Test was expected to abort but returned success.')
    exit(1)
elif not expected_abort and (retval != expected_exit_code):
    if expected_exit_code == 0:
        print('Test returned non-zero exit code (%d), full output below.' \
            % retval)
        print()
        print(output)
    else:
        print('Exit code %d does not match expected value of %d' \
            % (retval, expected_exit_code))
    exit(1)
exit(rc)
