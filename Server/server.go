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
	//"strings"
	"time"
)

const (
	kSize = 64
	kDeg  = 2
)

// TODO: fix this shamefulness
var (
	maps []terrainMap
)

func init() {
	rand.Seed(int64(time.Now().Nanosecond()))
	runtime.GOMAXPROCS(runtime.NumCPU())
}

func main() {
	// fmt.Println("generating terrain map of size", 32)
	// _ = genTerrainMap(32)
	// fmt.Println("terrain generation completed")
	// return
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
	defer conn.Close()
	// rd := bufio.NewReader(os.Stdin)
	wr := bufio.NewWriter(conn)

	// automatically write terrain map
	_ = genTerrainMap(kSize)
	for x := 0; x < kDeg; x++ {
		for y := 0; y < kDeg; y++ {
			// fmt.Println("sending terrain")
			wr.WriteString("terrain\n")
			fmt.Fprintln(wr, kSize/2)
			fmt.Fprintln(wr, x, y)
			buf := new(bytes.Buffer)
			tm := maps[x+y*2].arr
			err := binary.Write(buf, binary.LittleEndian, tm)
			if err != nil {
				fmt.Println(err)
			}
			io.Copy(wr, buf)
			wr.Flush()
			time.Sleep(time.Second)
		}
	}
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
	for i := 0; i < size; i++ {
		for j := 0; j < size; j++ {
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
	maps = tm.subdivide(kDeg)
	return tm.arr
}

type terrainMap struct {
	arr  []float32
	size int
}

func (tm *terrainMap) subdivide(deg int) []terrainMap {
	arr := make([]terrainMap, deg*deg, deg*deg)
	halfsize := tm.size / deg
	for i := range arr {
		arr[i].size = halfsize
		arr[i].arr = make([]float32, halfsize*halfsize, halfsize*halfsize)
	}
	for x := 0; x < tm.size; x++ {
		for y := 0; y < tm.size; y++ {
			tmX := x - x/halfsize
			tmY := y - y/halfsize
			val := tm.get(tmX, tmY)
			i := x / halfsize
			j := y / halfsize
			t := &arr[i+j*deg]
			// fmt.Println("tmX", tmX, "tmY", tmY, "i", i, "j", j,
			// 	"x", x%halfsize, "y", y%halfsize)
			t.set(x%halfsize, y%halfsize, val)
		}
	}
	return arr
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
		// p.arr[i].y = float32(size / 2)
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
