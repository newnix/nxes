#!/usr/bin/env es
# vim: ft=nxes

## Perform some simple arithmetic tests
## to validate the functionality of arithmetic,
## bitwise, and integer comparison primitives
myname = $0
tests = one-greater-zero zero-less-one sum-six

fn message {
	echo [$myname]: $*
}

fn run list {
	message Starting <={%count $list} tests...
	for (test = $list) {
		catch @ {
			if {~ $1 return} {
				if {~ $2 0} {pass = $pass $test}
				if {~ $2 1} {fail = $fail $test}
			} {
				fail = $fail $test
			}
		} {
			$test
		}
	}
	message Ran <={%count $tests}, Passed: <={%count $pass}, Failed: <={%count $fail}
	echo Passing tests: $pass
	if {%not {~ <={%count $fail} 0}} {
		echo Failed tests: $fail
	}
	return <={%count $fail}
}

fn one-greater-zero {
	message Test \'$0\' Expecting \{gt 1 0\} -\> true/0
	return <={$&greaterthan 1 0}
}

fn zero-less-one {
	message Test \'$0\' Expecting \{lt 0 1\} -\> true/0
	return <={$&lessthan 0 1}
}

fn sum-six {
	message Test \'$0\' Expecting \{sum 1 2 3\} -\> 6
	return <={~ {$&sum 1 2 3} 6}
}

run $tests
