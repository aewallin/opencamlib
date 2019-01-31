import Span from './span'

class Path {
    protected segments: Span[] = []

    append(line: Span) {
        this.segments.push(line)
    }

    serialize() {
        return this.segments.map(segment => segment.serialize())
    }
}

export default Path