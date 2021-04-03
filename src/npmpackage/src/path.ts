import Span from './span'

class Path {
    protected segments: Span[] = []

    append(segment: Span) {
        this.segments.push(segment)
    }

    serialize() {
        return this.segments.map(segment => segment.serialize())
    }
}

export default Path