package main

import (
	"bufio"
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"math/rand"
	"net"
	"os"
	"strings"
	"time"
)

const (
	kSize = 32
)

func init() {
	rand.Seed(int64(time.Now().Nanosecond()))
}

func main() {
	ln, err := net.Listen("tcp", ":1338")
	if err != nil {
		panic(err)
	}
	for {
		fmt.Println("waiting for connection")
		conn, err := ln.Accept()
		if err != nil {
			fmt.Print("Error accepting")
			continue
		}
		fmt.Println("Connection accepted")
		rd := bufio.NewReader(conn)
		go func() {
			io.Copy(os.Stdout, rd)
		}()
		go func() {
			sendCommands(conn)
		}()
	}
}

func sendCommands(conn net.Conn) {
	// rd := bufio.NewReader(os.Stdin)
	wr := bufio.NewWriter(conn)

	// automatically write terrain map
	tm := genTerrainMap(kSize)
	wr.WriteString("terrain\n")
	fmt.Fprintln(wr, kSize)
	buf := new(bytes.Buffer)
	_ = binary.Write(buf, binary.LittleEndian, tm)
	io.Copy(wr, buf)
	wr.Flush()

	// for {
	// 	fmt.Print("Enter command: ")
	// 	command, _ := rd.ReadString('\n')
	// 	command = strings.TrimSpace(command)
	// 	if command == "animate" {
	// 		command = animateCommand(rd)
	// 	} else if command == "animate preset" {
	// 		command = "animate 0 0 0 0 0 0 0 0 0 1 0 0 1 0 0 2 0 0 3 0 0 3 1 0 2 1 0 2 1 1 1 1 1 0 1 1 0 1 0 0 0 0 0 0 0"
	// 	}
	// 	_, err := wr.WriteString(command + "\n")
	// 	if err != nil {
	// 		fmt.Println("connection closed")
	// 		return
	// 	}
	// 	err = wr.Flush()
	// 	if err != nil {
	// 		fmt.Println("connection closed")
	// 		return
	// 	}
	// }
}

func animateCommand(rd *bufio.Reader) string {
	command := "animate"
	for {
		fmt.Print("Enter three points separated by spaces (or empty line to stop): ")
		line, _ := rd.ReadString('\n')
		if len(line) < 3 {
			break
		}
		command += " " + strings.TrimSpace(line)
	}
	return command
}

// terrain map generation
func genTerrainMap(size int) []float32 {
	tm := terrainMap{make([]float32, size*size, size*size), size}
	tm.initialize()
	var scale float32 = 1.0
	for size > 1 {
		tm.diamondSquare(size, scale)
		size /= 2
		scale /= 2.0
	}
	return tm.arr
}

type terrainMap struct {
	arr  []float32
	size int
}

func (tm *terrainMap) get(x, y int) float32 {
	return tm.arr[(x&(tm.size-1))+(y&(tm.size-1))*tm.size]
}

func (tm *terrainMap) set(x, y int, value float32) {
	tm.arr[(x&(tm.size-1))+(y&(tm.size-1))*tm.size] = value
}

func (tm *terrainMap) sampleSquare(x, y, size int, value float32) {
	hs := size / 2
	a := tm.get(x-hs, y-hs)
	b := tm.get(x+hs, y-hs)
	c := tm.get(x-hs, y+hs)
	d := tm.get(x+hs, y+hs)
	tm.set(x, y, (a+b+c+d)/4.+value)
}

func (tm *terrainMap) sampleDiamond(x, y, size int, value float32) {
	hs := size / 2
	a := tm.get(x-hs, y)
	b := tm.get(x+hs, y)
	c := tm.get(x, y-hs)
	d := tm.get(x, y+hs)
	tm.set(x, y, (a+b+c+d)/4.0+value)
}

func (tm *terrainMap) initialize() {
	for i := 0; i < tm.size; i += tm.size / 2 {
		for j := 0; j < tm.size; j += tm.size / 2 {
			tm.set(i, j, randFloat32())
		}
	}
}

func (tm *terrainMap) diamondSquare(stepsize int, scale float32) {
	halfstep := stepsize / 2
	for y := halfstep; y < tm.size+halfstep; y += stepsize {
		for x := halfstep; x < tm.size+halfstep; x += stepsize {
			tm.sampleSquare(x, y, stepsize, randFloat32()*scale)
		}
	}
	for y := 0; y < tm.size; y += stepsize {
		for x := 0; x < tm.size; x += stepsize {
			tm.sampleDiamond(x+halfstep, y, stepsize, randFloat32()*scale)
			tm.sampleDiamond(x, y+halfstep, stepsize, randFloat32()*scale)
		}
	}
}

func randFloat32() float32 {
	return rand.Float32()*2.0 - 1.0
}
