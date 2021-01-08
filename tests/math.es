#!/usr/bin/env es
# vim: ft=nxes

## Perform some simple arithmetic tests
## to validate the functionality of arithmetic,
## bitwise, and integer comparison primitives
myname = $0
tests = (one-greater-zero zero-less-one sum-six sub-six prod-six div-ten-two)
expect = (0 0 6 -6 6 5)

fn message {
	echo [$myname]: $*
}

fn check test result {
	got =<={$test}
	if {~ $got $result} {
		result true
	} {
		message \'$test\' expected: $result got: $got
		result false
	}
}

## This requires functioning primitives to run through the tests
## and tally them correctly, so this test script is definitely flawed
## but the script itself should also fail if the required primitives 
## don't function correctly. Additional tests will be needed.
fn run list {
	message Starting <={%count $list} tests...
	i =<={%count $tests}
	pass = 0; fail = 0
	while {$&greaterthan $i 0} {
		if {<={check $tests($i) $expect($i)}} {
			message \'$tests($i)\': Pass
			pass =<={$&sum $pass 1}
		} {
			message \'$tests($i)\': Fail
			fail =<={$&sum $fail 1}
		}
		i =<={$&sub $i 1}
	}
	message Ran <={%count $tests}, Passed: $pass, Failed: $fail
	return $fail
}

fn one-greater-zero {
	return <={$&greaterthan 1 0}
}

fn zero-less-one {
	return <={$&lessthan 0 1}
}

fn sum-six {
	return <={$&sum 1 2 3}
}

fn sub-six {
	return <={$&sub 0 1 2 3}
}

fn prod-six {
	return <={$&mul 1 2 3}
}

fn div-ten-two {
	return <={$&div 10 2}
}

run $tests
