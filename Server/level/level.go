package level

import (
	"../twodtree"
	"bufio"
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"math"
	"math/rand"
)

// define difficulty type
type Difficulty int

// levels of difficulty
const (
	EASY Difficulty = iota
	MEDIUM
	HARD
)

// size parameters
const (
	kSize        = 1024 // size of total map
	kDeg         = 16   // number of times to subdivide each side
	kEasyShips   = 20
	kMediumShips = 40
	kHardShips   = 60
)

// commands
const (
	TERRAIN    = "terrain"
	PATH       = "path"
	DONE       = "done"
	READY      = "ready"
	START      = "start"
	ENEMY_SHIP = "enemy_ship"
)

var (
	easyLevelChan   chan *Level
	mediumLevelChan chan *Level
	hardLevelChan   chan *Level
)

func init() {
	easyLevelChan = make(chan *Level, 2)
	go func() {
		for {
			maps, p := genTerrainMap(kSize)
			ships := genShips(kEasyShips)
			easyLevelChan <- &Level{maps, p, ships}
		}
	}()
	mediumLevelChan = make(chan *Level, 2)
	go func() {
		for {
			maps, p := genTerrainMap(kSize)
			ships := genShips(kMediumShips)
			mediumLevelChan <- &Level{maps, p, ships}
		}
	}()
	hardLevelChan = make(chan *Level, 2)
	go func() {
		for {
			maps, p := genTerrainMap(kSize)
			ships := genShips(kHardShips)
			hardLevelChan <- &Level{maps, p, ships}
		}
	}()
}

// represents a struct containing terrainMaps and a path
type Level struct {
	maps  []terrainMap
	p     path
	ships []ship
}

func GetLevel(d Difficulty) (l *Level) {
	switch d {
	case EASY:
		return <-easyLevelChan
	case MEDIUM:
		return <-mediumLevelChan
	case HARD:
		return <-hardLevelChan
	}
	return
}

func (l *Level) WriteData(wr *bufio.Writer) {
	maps := l.maps
	p := l.p

	for x := 0; x < kDeg; x++ {
		for y := 0; y < kDeg; y++ {
			fmt.Fprintln(wr, TERRAIN)
			fmt.Fprintln(wr, maps[x+y*kDeg].size, x, y)
			buf := new(bytes.Buffer)
			tm := maps[x+y*kDeg].arr
			err := binary.Write(buf, binary.LittleEndian, tm)
			if err != nil {
				fmt.Println(err)
			}
			io.Copy(wr, buf)
		}
	}
	for _, s := range l.ships {
		fmt.Fprintln(wr, ENEMY_SHIP)
		fmt.Fprintln(wr, s.timeOffset, s.offset.x, s.offset.y)
	}
	fmt.Fprintln(wr, PATH)
	pathArr := sampleArr(p.arr, 14) // TODO: change to 14 * 4
	fmt.Fprintln(wr, len(pathArr))
	buf := new(bytes.Buffer)
	binary.Write(buf, binary.LittleEndian, pathArr)
	io.Copy(wr, buf)
	fmt.Fprintln(wr, DONE)
	wr.Flush()
	fmt.Println("Finished sending Level over network")
}

type ship struct {
	timeOffset float32
	offset     vec2
}

func genShips(numShips int) []ship {
	ships := make([]ship, numShips)
	for i := range ships {
		ships[i].offset.x = randFloat32() * 2.4
		ships[i].offset.y = randFloat32() * 1.8
		ships[i].timeOffset = rand.Float32() * 70
	}
	return ships
}

// samples a slice in groups of size step and averages out the
// values. This is to smooth out the path the ship will follow
// to make it less jerky
func sampleArr(arr []vec3, step int) []vec3 {
	newArr := make([]vec3, 0)
	for i := 0; i < len(arr); i += step {
		var v vec3
		slice := arr[i:]
		if len(slice) > step {
			slice = slice[:step]
		}
		for _, val := range slice {
			v.x += val.x
			v.y += val.y
			v.z += val.z
		}
		v.x /= float32(len(slice))
		v.y /= float32(len(slice))
		v.z /= float32(len(slice))
		newArr = append(newArr, v)
	}
	return newArr
}

func canyon(d float64) float32 {
	// y=1/(10 * sqrt(2*pi)) * e^(-x^2/(2*10*10))
	sigma := 10.0
	a := 1.0 / (sigma * math.Sqrt(2.0*math.Pi))
	exp := a * math.Exp(-d*d/(2.0*sigma*sigma)) * 7.0
	// fmt.Println(float32(1.0 - exp))
	return float32(exp)
}

// terrain map generation, with mutliple return values
func genTerrainMap(size int) ([]terrainMap, path) {
	// declares a terrainMap on the stack
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
	p := genPath(size)
	for i := 0; i < size; i++ {
		for j := 0; j < size; j++ {
			dist := float64(p.distanceTo(float32(i), float32(j)))
			// if dist < 10.0 {
			// tm.set(i, j, tm.get(i, j)-float32(math.Sqrt(dist+1)/15.))
			tm.set(i, j, tm.get(i, j)-canyon(dist))
			// tm.set(i, j, tm.get(i, j)-1000.0)
			// }
		}
	}
	tm.normalize()
	p.adjustHeights(&tm)
	for i := 0; i < len(p.arr); i++ {
		// p.arr[i].x /= float32(kSize/kDeg + float32(int(p.arr[i].x)/kDeg))
		// p.arr[i].y /= float32(kSize/kDeg + float32(int(p.arr[i].y)/kDeg))
		p.arr[i].x /= float32((kSize - 1) / kDeg)
		p.arr[i].y /= float32((kSize - 1) / kDeg)
	}
	maps := tm.subdivide(kDeg)
	return maps, p
}

// a height map is represented by a slice of heights
// note that the size should be a power of two
type terrainMap struct {
	arr  []float32
	size int
}

// an example of a method in Go. After the func keyword, the type
// that the method is being called on is declared.
// this method subdivides a terrainMap into different sub maps
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
			t.set(x%halfsize, y%halfsize, val)
		}
	}
	return arr
}

// this index method allows us to index into a terrainMap with overlap
func (tm *terrainMap) index(x, y int) int {
	return (x & (tm.size - 1)) + (y&(tm.size-1))*tm.size
}

// gets the value of the terrainMap at x, y
func (tm *terrainMap) get(x, y int) float32 {
	return tm.arr[tm.index(x, y)]
}

// sets the value of the terrainMap at x, y
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

// runs a step of the Diamond Square algorithm for procedurally
// generating height maps
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

// normalizes a terrainMap to be between 0 and 1
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

// returns a random float32 between -1 and 1
func randFloat32() float32 {
	return rand.Float32()*2.0 - 1.0
}

type vec3 struct {
	x, y, z float32
}

type vec2 struct {
	x, y float32
}

func (v vec3) distanceTo(o vec3) float32 {
	return float32(math.Sqrt(float64((v.x-o.x)*(v.x-o.x) + (v.y-o.y)*(v.y-o.y))))
}

type path struct {
	arr  []vec3
	tree twodtree.TwoDTree
}

// creates a procedurally generated path using midpoint displacement
func genPath(size int) path {
	p := path{make([]vec3, size*2, size*2), twodtree.TwoDTree{}}
	for i := 0; i < len(p.arr); i++ {
		p.arr[i].x = float32(i)*0.5 + 0.5
		p.arr[i].y = float32(size / 2)
	}
	size /= 2
	for size > 1 {
		p.line(size, float32(size)/4.)
		size /= 2
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
	return p
}

func (p *path) distanceTo(x, y float32) float32 {
	// o := vec3{x, y, 0}
	// minDist := o.distanceTo(p.arr[0])
	// for _, v := range p.arr {
	// 	if v.distanceTo(o) < minDist {
	// 		minDist = v.distanceTo(o)
	// 	}
	// }
	// return minDist
	closest := p.tree.NearestNeighbor([2]float32{x, y})
	return (vec3{x, y, 0}).distanceTo(vec3{closest[0], closest[1], 0})
}

// these methods are the 1D counterpart of the diamond square
// methods above
func (p *path) index(i int) int {
	return i % len(p.arr)
}

func (p *path) get(i int) float32 {
	return p.arr[p.index(i)].y
}

func (p *path) set(i int, value float32) {
	p.arr[p.index(i)].y = value
}

func (p *path) line(stepsize int, scale float32) {
	halfstep := stepsize / 2
	for i := halfstep; i < len(p.arr); i += stepsize {
		p.sampleLine(i, stepsize, randFloat32()*scale)
	}
}

func (p *path) sampleLine(i, size int, value float32) {
	hs := size / 2
	a := p.get(i + hs)
	b := p.get(i - hs)
	p.set(i, (a+b)/2.0+value)
}

func (p *path) adjustHeights(tm *terrainMap) {
	for i := range p.arr {
		x := int(p.arr[i].x)
		y := int(p.arr[i].y)
		if x < len(p.arr)-1 && y < len(p.arr)-1 {
			s := p.arr[i].x - float32(x)
			t := p.arr[i].y - float32(y)
			v0 := tm.get(x, y)
			v1 := tm.get(x+1, y)
			v2 := tm.get(x, y+1)
			v3 := tm.get(x+1, y+1)
			p.arr[i].z = bilerp(s, t, v0, v1, v2, v3)
		} else {
			p.arr[i].z = tm.get(x, y)
		}
	}
}

func lerp(t, v0, v1 float32) float32 {
	return (1-t)*v0 + t*v1
}

func bilerp(s, t, v0, v1, v2, v3 float32) float32 {
	v01 := lerp(s, v0, v1)
	v23 := lerp(s, v2, v3)
	return lerp(t, v01, v23)
}
