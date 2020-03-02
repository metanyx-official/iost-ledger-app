package main

import (
    "fmt"
	"time"
	"tx/tx"
)

func main() {
	t := time.Now()
	tx := tx.NewTx();
    fmt.Printf("%d\n", t.Month())
}
