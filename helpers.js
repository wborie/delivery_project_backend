// note that this implementation of priority_queue does not allow for vlogv dijkstra, it does v^2 (doesnt use a heap)
class priority_queue {
  constructor() {
    this.queue = []; // an array of elements where elements are of form {element: object, weight: Number}
  }

  // pushes element onto the priority queue (lower priority = earlier in priority queue)
  push(element, priority) {
    let pushed = false;
    for(let i = 0; i < this.queue.length; i++) {
      if (this.queue[i].priority > priority) {
        this.queue.splice(i, 0, {element: element, priority: priority});
        pushed = true;
        break;
      }
    }
    if (!pushed) {
      this.queue.push({element: element, priority: priority});
    }
  }

  // update_priority(element, newPriority) {
  //   this.queue.forEach((value, index) => {
  //     if (value === element) {
  //       this.queue.splice(index, 1);
  //       this.push(element, newPriority);
  //     }
  //   })
  // }

  pop() {
    return this.queue.shift().element;
  }

  isEmpty() {
    return !this.queue.length;
  }

  print() {
    this.queue.forEach(element => {console.log(element.element, "| priority:", element.priority,);})
  }
}

class Graph {
  constructor() {
    this.vertices = [];
    this.edges = {};
  }

  addVertex(vertex) {
    this.vertices.push(vertex);
    this.edges[vertex] = [];
  }

  addEdge(startVertex, endVertex, weight) {
    this.edges[startVertex].push({start: startVertex, end: endVertex, weight: weight});
  }

  getVertices() {
    return this.vertices;
  }

  getEdges(vertex) {
    return this.edges[vertex];
  }


  print() {
    let result = ``;
    for(let i = 0; i < this.vertices.length; i++) {
      result += `Vertex ${this.vertices[i]} has edges`;
      this.edges[this.vertices[i]].forEach(edge => {
        result += ` ${edge.start}->${edge.end}:${edge.weight}`;
      })
      result += `\n`;
    }
    console.log(result);
  }
}

function objectValuesAreEqual(firstObject, secondObject) {
  objectsAreEqual = true;
  Object.keys(firstObject).forEach(key => {
    if (firstObject[key] !== secondObject[key]) objectsAreEqual = false;
  })
  return objectsAreEqual;
}

module.exports = {
  priority_queue: priority_queue,
  Graph: Graph,
  objectValuesAreEqual: objectValuesAreEqual
}