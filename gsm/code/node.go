package code

import "fmt"

type NodeType int

const (
	NT_ABSOLUTE NodeType = iota
	NT_RELOCATABLE
)

type fnIdx struct {
	spanIdx *span
	offset  uint32
}

type span struct {
	baddr   uint32
	code    []byte
	next    *span
	fnTable map[string]uint32
}

type Node struct {
	name    string
	nType   NodeType
	head    *span
	tail    *span
	fnTable map[string]fnIdx
}

func NewNode(name string, nType NodeType) *Node {
	return &Node{
		name:    name,
		nType:   nType,
		head:    nil,
		tail:    nil,
		fnTable: make(map[string]fnIdx),
	}
}

func validateSpanFnTable(cLen uint32, fnTable map[string]uint32) error {
	for _, offset := range fnTable {
		if offset%4 != 0 {
			return fmt.Errorf("Offset for function %q is not word aligned: %x", offset)
		}
		if offset >= uint32(cLen) {
			return fmt.Errorf("Offset ofr function %q is larger than code span: %x >= %x", offset, cLen)
		}
	}
	return nil
}

func (n *Node) AddSpan(baddr uint32, code []byte, fnTable map[string]uint32) error {
	if err := validateSpanFnTable(uint32(len(code)), fnTable); err != nil {
		return err
	}

	s := &span{
		baddr:   baddr,
		code:    code,
		fnTable: fnTable,
		next:    nil,
	}

	// Easy case: first insertion
	if n.head == nil {
		n.tail = s
		n.head = n.tail
		return nil
	}

	// Common case: new span to existing span list. Need to check if baddr is valid.
	bLen := n.tail.baddr + uint32(len(n.tail.code))
	if bLen > baddr {
		return fmt.Errorf("New span overlaps or starts before previous span: %x > %x", bLen, baddr)
	}

	// Since there is no overlap, we can safely add the span to the node.
	n.tail.next = s
	n.tail = s
	return nil
}

func (n *Node) PackSpan(code []byte, fnTable map[string]uint32) error {
	if err := validateSpanFnTable(uint32(len(code)), fnTable); err != nil {
		return err
	}
	s := &span{
		code:    code,
		fnTable: fnTable,
		next:    nil,
	}

	// First insertion.
	if n.head == nil {
		n.tail = s
		n.head = n.tail
		return nil
	}

	// Common case: new span to existing span list.
	s.baddr = n.tail.baddr + uint32(len(n.tail.code))
	n.tail.next = s
	n.tail = s
	return nil
}

func (n *Node) SpanCount() int {
	count := 0
	for s := n.head; s != nil; s = s.next {
		count++
	}
	return count
}

func (n *Node) LiftFuncTable() {
	if len(n.fnTable) > 0 {
		return
	}
	for s := n.head; s != nil; s = s.next {
		for fn, offset := range s.fnTable {
			n.fnTable[fn] = fnIdx{
				spanIdx: s,
				offset:  offset,
			}
		}
		s.fnTable = nil
	}
}

func (n *Node) ShiftSpan(shift uint32) {
	for s := n.head; s != nil; s = s.next {
		s.baddr += shift
	}
}

func (n *Node) MergeAfter(s *span) {
	aux := s.next
	s.next = n.head
	n.tail.next = aux
}

func (n *Node) NormalizeAddresses() {
}
