from mtree.tests.fixtures.generator import ADD, REMOVE, QUERY
"""
actions = '4a4r4a4r'
dimensions = 2
remove_chance = 0.1
"""

DIMENSIONS = 2

def PERFORM(callback):
	callback(ADD((5, 40), QUERY((45, 84), 15.412622811172305, 3)))
	callback(ADD((33, 13), QUERY((32, 89), 64.27250089810116, 1)))
	callback(ADD((19, 41), QUERY((79, 47), 7.461019616498659, 6)))
	callback(ADD((31, 14), QUERY((12, 31), 72.61265409866988, 8)))
	callback(REMOVE((31, 14), QUERY((24, 35), 72.19210409206508, 4)))
	callback(REMOVE((5, 40), QUERY((18, 70), 74.61455642731022, 2)))
	callback(REMOVE((33, 13), QUERY((31, 81), 48.444812726212035, 1)))
	callback(REMOVE((19, 41), QUERY((53, 90), 24.961800751634087, 4)))
	callback(ADD((21, 33), QUERY((28, 50), 63.77859214760758, 3)))
	callback(ADD((88, 79), QUERY((52, 65), 78.68804088171697, 5)))
	callback(ADD((59, 26), QUERY((64, 71), 75.56923568579985, 1)))
	callback(ADD((92, 16), QUERY((99, 100), 0.12428365403171604, 2)))
	callback(REMOVE((59, 26), QUERY((41, 20), 70.26109949345349, 2)))
	callback(REMOVE((92, 16), QUERY((88, 4), 33.23891390579037, 7)))
	callback(REMOVE((88, 79), QUERY((12, 11), 37.7894432108786, 5)))
	callback(REMOVE((21, 33), QUERY((24, 61), 76.72849288348556, 0)))
