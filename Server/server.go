// the main package is where Go programs start
package main

// as an effort to speed compilation and prevent bad programming
// practices, Go requires all imported packages to be used
import (
	"./level"
	"bufio"
	"fmt"
	"io"
	"math/rand"
	"net"
	"runtime"
	"strings"
	"time"
)

// size parameters
const (
	kSize = 1024 // size of total map
	kDeg  = 16   // number of times to subdivide each side
)

// commands
const (
	TERRAIN = "terrain"
	PATH    = "path"
	DONE    = "done"
	READY   = "ready"
	START   = "start"
	END     = "end"
	KEY     = "key"
	BULLET  = "bullet"
)

// the init function will get called before main() gets called
func init() {
	// Go's rand package requires the seed to be set
	// rand is a package, and Seed is a function in that package
	// the capitalization of a function, variable, or constant in
	// a package denotes whether or not it gets exported
	rand.Seed(int64(time.Now().Nanosecond()))

	// Sets how many threads Go will use. Note that a goroutine
	// does not necessarily correspond with a thread
	runtime.GOMAXPROCS(runtime.NumCPU())
}

func main() {
	// this opens up a new tcp connection on port 1338
	// note the multiple return values, it's idiomatic Go
	// to return an error value after the regular return value
	// if a function may fail

	// the := operator is duck typing, and automatically sets
	// the types of ln and err to the appropriate types
	ln, err := net.Listen("tcp", ":3789")
	if err != nil {
		panic(err)
	}

	oneToTwo := make(chan string)
	twoToOne := make(chan string)

	var lastLevel *level.Level
	var lastServer *server
	// var lastPlayer string

	// no while loops in Go, just variations of the for loop:
	// for with nothing -- like for (;;)
	// for with one condition -- like a while loop
	// for with three statements -- like a for loop
	// for with a range statement -- like a foreach loop
	for {
		fmt.Println("waiting for connection")

		// the conn object is the actual tcp connection
		conn, err := ln.Accept()
		if err != nil {
			fmt.Println("Error accepting")
			fmt.Println(err)
			continue
		}
		fmt.Println("Connection accepted")

		rd := bufio.NewReader(conn)
		wr := bufio.NewWriter(conn)

		// the "go" keyword starts a new goroutine, the goroutine
		// will be executed concurrently
		go func() {
			player, err := rd.ReadString('\n')
			if err != nil {
				fmt.Println("Error reading from connection")
				fmt.Println(err)
				return
			}
			player = strings.TrimSpace(player)
			player = strings.ToLower(player)

			// respond to ping request
			if player == "ping" {
				fmt.Fprintln(wr, "pong")
				wr.Flush()
				conn.Close()
				return
			}

			// next thing sent is difficulty
			difficulty, _ := rd.ReadString('\n')
			difficulty = strings.TrimSpace(difficulty)
			fmt.Println("difficulty:", difficulty)

			if lastLevel == nil || player == "1" ||
				(len(player) == 2 && player[1] == 's') {
				// if lastLevel == nil {
				var diff level.Difficulty
				switch difficulty {
				case "easy":
					diff = level.EASY
				case "medium":
					diff = level.MEDIUM
				case "hard":
					diff = level.HARD
				}
				lastLevel = level.GetLevel(diff)
				lastServer = new(server)
				lastServer.l = lastLevel
				// lastLevel = nil
			}

			lastLevel.WriteData(wr)
			// lastServer.l.WriteData(wr)
			lastServer.addClient(conn, rd, wr, player == "1" || player == "1s")

			if len(player) == 2 {
				synchronizeStart(rd, wr, player, nil, nil)
			} else if player == "1" {
				synchronizeStart(rd, wr, player, oneToTwo, twoToOne)
			} else {
				synchronizeStart(rd, wr, player, twoToOne, oneToTwo)
			}

			if player != "2" {
				lastServer.run()
			}
		}()
		runtime.GC()
	}
}

// right now, this function generates a terrain map and sends
// it over the network
func synchronizeStart(rd *bufio.Reader, wr *bufio.Writer, player string, send, receive chan string) {
	_, _ = rd.ReadString('\n') // this should by READY
	if player == "1" {
		send <- READY
	} else if player == "2" {
		<-receive
	}
	fmt.Fprintln(wr, START)
	wr.Flush()
	fmt.Println("start command for player", player, "sent")
}

// contains all the data associated with a server
type server struct {
	one, two    *client // the two players
	oneToServer chan string
	twoToServer chan string
	serverToOne chan string
	serverToTwo chan string
	l           *level.Level
}

// represents a client that's connected to the server
type client struct {
	conn    net.Conn
	rd      *bufio.Reader
	wr      *bufio.Writer
	outChan chan string
}

// adds a client to the server
func (s *server) addClient(conn net.Conn, rd *bufio.Reader, wr *bufio.Writer, isOne bool) {
	c := &client{conn, rd, wr, make(chan string, 5)}
	if isOne {
		s.one = c
		s.oneToServer = make(chan string, 5)
		s.serverToOne = make(chan string, 5)
	} else {
		s.two = c
		s.twoToServer = make(chan string, 5)
		s.serverToTwo = make(chan string, 5)
	}
}

// main running method for server
func (s *server) run() {
	if s.one != nil {
		go copyToWriter(s.one.conn, s.one.outChan, s.one.wr)
		go copyToChan(s.one.rd, s.oneToServer)
	}
	if s.two != nil {
		go copyToWriter(s.two.conn, s.two.outChan, s.two.wr)
		go copyToChan(s.two.rd, s.twoToServer)
	}

	var line string
	for {
		select {
		case line = <-s.oneToServer:
			switch strings.Split(line, " ")[0] {
			case END:
				if s.two != nil {
					s.two.outChan <- line
				}
				s.one.outChan <- line
				return
			default:
				if s.two != nil {
					s.two.outChan <- line
				}
			}
		case line = <-s.twoToServer:
			switch strings.Split(line, " ")[0] {
			case END:
				if s.one != nil {
					s.one.outChan <- line
				}
				s.two.outChan <- line
				return
			default:
				if s.one != nil {
					s.one.outChan <- line
				}
			}
		}
	}
}

func copyToWriter(conn net.Conn, ch chan string, wr *bufio.Writer) {
	for {
		command := <-ch
		fmt.Fprintln(wr, command)
		wr.Flush()
		if command == END {
			conn.Close()
			break
		}
	}
}

func copyToChan(rd *bufio.Reader, ch chan string) {
	for {
		str, err := rd.ReadString('\n')
		if err == io.EOF {
			ch <- END
			break
		} else if err != nil {
			fmt.Println("Error copying to chan from stream")
			ch <- END
			break
		}
		str = strings.TrimSpace(str)
		ch <- str
	}
}
