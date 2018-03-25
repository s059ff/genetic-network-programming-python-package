import os
import unittest

import gnp


class TestGenomeSerialization(unittest.TestCase):

    def test_serialization(self):
        config = gnp.GNPConfig('gnp-config.json')

        # Create and randomly initialize genome.
        genome1 = gnp.Genome()
        genome1.configure_new(config)

        # Serialize to a file.
        genome1.serialize('genome.json', config)

        # Deserealize from a file.
        genome2 = gnp.Genome()
        genome2.deserialize('genome.json', config)

        # Check if serialized object is equal to deserialized object.
        self.assertEqual(genome1, genome2)


class TestPopulationSerialization(unittest.TestCase):

    def test_serialization(self):
        config = gnp.GNPConfig('gnp-config.json')

        # Create and randomly initialize population.
        population1 = gnp.Population(config)

        # Serialize to a file.
        population1.serialize('population.json', config)

        # Deserealize from a file.
        population2 = gnp.Population(config)
        population2.deserialize('population.json', config)

        # Check if serialized object is equal to deserialized object.
        self.assertEqual(population1, population2)


if __name__ == '__main__':
    os.chdir(os.path.dirname(__file__))
    unittest.main()
