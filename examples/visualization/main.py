import os

import gnp
import pydotplus


def main():
    config = gnp.GNPConfig('gnp-config.json')

    # Create and randomly initialize genome.
    genome = gnp.Genome()
    genome.configure_new(config)

    # Save the figure as dot format file.
    genome.savefig('genome.dot', config)

    # Convert dot file to png.
    graph = pydotplus.graphviz.graph_from_dot_file('genome.dot')
    graph.write('genome.png', format='png')


if __name__ == '__main__':
    os.chdir(os.path.dirname(__file__))
    main()
