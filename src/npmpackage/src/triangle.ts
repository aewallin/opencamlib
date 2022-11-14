import Point from './point'

class Triangle {
  constructor(protected p1: Point, protected p2: Point, protected p3: Point) {}

  serialize(): [string, any[]] {
    return ['Triangle', [this.p1, this.p2, this.p3]]
  }
}

export default Triangle