// Implements a two dimensional tree for float32s
// one of the downsides of Go is the lack of generics or templates.
// The designers didn't want the compilation costs associated with
// templates or the runtime costs associated with generics, so they
// decides to just leave it out. As a result, this package just
// implements a two dimensional kdtree for float32s
// However, it would be easy to make it a kdtree of float32s by
// just using slices instead of fixed sized arrays
package twodtree

import (
	"math"
)

type TwoDTree struct {
	root *node
}

// exported Add method, note the capitalizatoin
func (tree *TwoDTree) Add(elem [2]float32) {
	if tree.root == nil {
		tree.root = &node{elem, nil, nil, nil}
	} else {
		tree.add(elem, tree.root, 0)
	}
}

// unexported recursive add
func (tree *TwoDTree) add(elem [2]float32, curr *node, count int) {
	index := count % 2
	if elem[index] < curr.elem[index] {
		// go to left side
		if curr.left == nil {
			curr.left = &node{elem, nil, nil, curr}
		} else {
			tree.add(elem, curr.left, count+1)
		}
	} else {
		// go to right side
		if curr.right == nil {
			curr.right = &node{elem, nil, nil, curr}
		} else {
			tree.add(elem, curr.right, count+1)
		}
	}
}

func (tree *TwoDTree) NearestNeighbor(test [2]float32) [2]float32 {
	g := guess{nil, test, float32(math.Inf(1))}
	g.procedure(tree.root, 0, false)
	return g.bestGuess.elem
}

type node struct {
	elem        [2]float32
	left, right *node
	parent      *node
}

func distance(one, two [2]float32) float32 {
	sqsum := (one[0]-two[0])*(one[0]-two[0]) + (one[1]-two[1])*(one[1]-two[1])
	return float32(math.Sqrt(float64(sqsum)))
}

type guess struct {
	bestGuess *node
	test      [2]float32
	bestDist  float32
}

// nearest neighbor lookup procedure
func (g *guess) procedure(curr *node, count int, visitedOtherSide bool) {
	if curr == nil {
		return
	}
	dist := distance(curr.elem, g.test)
	if dist < g.bestDist {
		g.bestGuess = curr
		g.bestDist = dist
	}
	index := count % 2
	if g.test[index] < curr.elem[index] {
		g.procedure(curr.left, count+1, false)
	} else {
		g.procedure(curr.right, count+1, false)
	}
	index = (count + 1) % 2
	if !visitedOtherSide &&
		curr.parent != nil &&
		math.Abs(float64(curr.elem[index]-g.test[index])) < float64(g.bestDist) {
		if curr.parent.left == curr {
			g.procedure(curr.parent.right, count, true)
		} else {
			g.procedure(curr.parent.left, count, true)
		}
	}
}
