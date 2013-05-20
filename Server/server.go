package main

import (
	"./twodtree"
	"bufio"
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"math"
	"math/rand"
	"net"
	"os"
	"runtime"
	"strings"
	"time"
)

const (
	kSize = 32
)

func init() {
	rand.Seed(int64(time.Now().Nanosecond()))
	runtime.GOMAXPROCS(runtime.NumCPU())
}

func main() {
	go func() {
		fmt.Println("generating terrain map of size", 1024)
		_ = genTerrainMap(1024)
		fmt.Println("terrain generation completed")
	}()
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
	err := binary.Write(buf, binary.LittleEndian, tm)
	if err != nil {
		fmt.Println(err)
	}
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
	t0 := time.Now()
	tm := terrainMap{make([]float32, size*size, size*size), size}
	tm.initialize()
	var scale float32 = 1
	initSize := size
	size /= 2
	for size > 1 {
		tm.diamondSquare(size, scale)
		size /= 2
		scale /= 2.0
	}
	size = initSize
	t1 := time.Now()
	p := genPath(size)
	// increment := float32(1) / float32(size)
	for i := 0; i < size; i++ {
		for j := 0; j < size; j++ {
			// x := float32(i) * increment
			// y := float32(j) * increment
			dist := float64(p.distanceTo(float32(i), float32(j)))
			if dist < 3.0 {
				tm.set(i, j, tm.get(i, j)-float32(math.Sqrt(4.0-dist)))
			}
		}
	}
	t2 := time.Now()
	tm.normalize()
	t3 := time.Now()
	fmt.Println("Initial algorithm took", t1.Sub(t0))
	fmt.Println("Path calculations took", t2.Sub(t1))
	fmt.Println("tm normalizations took", t3.Sub(t2))
	return tm.arr
}

type terrainMap struct {
	arr  []float32
	size int
}

func (tm *terrainMap) index(x, y int) int {
	return (x & (tm.size - 1)) + (y&(tm.size-1))*tm.size
}

func (tm *terrainMap) get(x, y int) float32 {
	return tm.arr[tm.index(x, y)]
}

func (tm *terrainMap) set(x, y int, value float32) {
	tm.arr[tm.index(x, y)] = value
}

func (tm *terrainMap) initialize() {
	for i := 0; i < tm.size; i += tm.size / 2 {
		for j := 0; j < tm.size; j += tm.size / 2 {
			tm.set(i, j, randFloat32())
		}
	}
}

func (tm *terrainMap) sampleSquare(x, y, size int, value float32) {
	hs := size / 2
	a := tm.get(x-hs, y-hs)
	b := tm.get(x+hs, y-hs)
	c := tm.get(x-hs, y+hs)
	d := tm.get(x+hs, y+hs)
	tm.set(x, y, (a+b+c+d)/4.0+value)
}

func (tm *terrainMap) sampleDiamond(x, y, size int, value float32) {
	hs := size / 2
	a := tm.get(x-hs, y)
	b := tm.get(x+hs, y)
	c := tm.get(x, y-hs)
	d := tm.get(x, y+hs)
	tm.set(x, y, (a+b+c+d)/4.0+value)
}

func (tm *terrainMap) diamondSquare(stepsize int, scale float32) {
	halfstep := stepsize / 2
	for y := halfstep; y < tm.size; y += stepsize {
		for x := halfstep; x < tm.size; x += stepsize {
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

func (tm *terrainMap) normalize() {
	min := tm.arr[0]
	max := min
	for i := 1; i < len(tm.arr); i++ {
		if tm.arr[i] < min {
			min = tm.arr[i]
		}
		if tm.arr[i] > max {
			max = tm.arr[i]
		}
	}
	diff := max - min
	for i := 0; i < len(tm.arr); i++ {
		tm.arr[i] -= min
		tm.arr[i] /= diff
	}
}

func randFloat32() float32 {
	return rand.Float32()*2.0 - 1.0
}

type vec2 struct {
	x, y float32
}

func (v vec2) distanceTo(o vec2) float32 {
	return float32(math.Sqrt(float64((v.x-o.x)*(v.x-o.x) + (v.y-o.y)*(v.y-o.y))))
}

type path struct {
	arr  []vec2
	tree twodtree.TwoDTree
}

func genPath(size int) path {
	p := path{make([]vec2, size-1, size-1), twodtree.TwoDTree{}}
	for i := 0; i < len(p.arr); i++ {
		p.arr[i].x = float32(i) + 0.5
		p.arr[i].y = float32(i) + 0.5
	}
	arr := make([]int, len(p.arr), len(p.arr))
	for i := range arr {
		arr[i] = i
	}
	for i := 0; i < len(arr)*4; i++ {
		idx := i % len(arr)
		other := rand.Intn(len(arr))
		arr[idx], arr[other] = arr[other], arr[idx]
	}
	for _, i := range arr {
		p.tree.Add([2]float32{p.arr[i].x, p.arr[i].y})
	}
	fmt.Println("added all elements")
	return p
}

func (p *path) distanceTo(x, y float32) float32 {
	// o := vec2{x, y}
	// minDist := o.distanceTo(p.arr[0])
	// for _, v := range p.arr {
	// 	if v.distanceTo(o) < minDist {
	// 		minDist = v.distanceTo(o)
	// 	}
	// }
	// return minDist
	closest := p.tree.NearestNeighbor([2]float32{x, y})
	return (vec2{x, y}).distanceTo(vec2{closest[0], closest[1]})
}
