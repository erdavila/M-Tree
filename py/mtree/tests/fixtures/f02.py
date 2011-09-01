from mtree.tests.fixtures.generator import ADD, REMOVE, QUERY
"""
actions = '2a2r2a2r'
dimensions = 3
remove_chance = 0.1
"""

DIMENSIONS = 3

def PERFORM(callback):
	callback(ADD((13, 77, 34), QUERY((94, 23, 53), 20.937134443888432, 6)))
	callback(ADD((47, 20, 27), QUERY((55, 83, 49), 46.148506220254355, 6)))
	callback(REMOVE((47, 20, 27), QUERY((38, 10, 55), 33.026138417023965, 3)))
	callback(REMOVE((13, 77, 34), QUERY((70, 16, 22), 13.524337475744934, 0)))
	callback(ADD((88, 80, 82), QUERY((19, 43, 49), 66.72013879162282, 4)))
	callback(ADD((90, 14, 7), QUERY((2, 30, 12), 28.401279559773034, 5)))
	callback(REMOVE((88, 80, 82), QUERY((18, 31, 46), 6.763919170829018, 1)))
	callback(REMOVE((90, 14, 7), QUERY((47, 27, 8), 50.78903456072281, 5)))
