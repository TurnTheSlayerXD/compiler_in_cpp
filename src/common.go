package main

import (
	"fmt"
)


func panic_fmt(format string, args ...any) {
	panic(fmt.Sprintf(format, args))
}

func assert(t bool, msg string) {
	if (!t) {
		panic(msg)
	}
}
