// Implements a two dimensional tree for float32s
package twodtree

import (
	// "fmt"
	"math"
)

type TwoDTree struct {
	root *node
}

func (tree *TwoDTree) Add(elem [2]float32) {
	if tree.root == nil {
		tree.root = &node{elem, nil, nil, nil}
	} else {
		tree.add(elem, tree.root, 0)
	}
}

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
