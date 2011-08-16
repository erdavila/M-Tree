from mtree.tests.fixtures.generator import ADD, REMOVE, QUERY
"""
actions = '5a5r5a5r'
dimensions = 2
remove_chance = 0.1
"""

DIMENSIONS = 2

def PERFORM(callback):
	callback(ADD((44, 83), QUERY((39, 97), 59.750079860371876, 6)))
	callback(ADD((23, 0), QUERY((88, 56), 22.88267964697555, 3)))
	callback(ADD((52, 10), QUERY((44, 27), 24.564897258652756, 8)))
	callback(ADD((95, 80), QUERY((66, 44), 33.31525230748455, 1)))
	callback(ADD((55, 42), QUERY((64, 60), 68.99205324052721, 4)))
	callback(REMOVE((52, 10), QUERY((45, 7), 9.173642113746991, 3)))
	callback(REMOVE((55, 42), QUERY((39, 96), 65.54009599782735, 6)))
	callback(REMOVE((23, 0), QUERY((5, 43), 1.952301791228388, 4)))
	callback(REMOVE((44, 83), QUERY((13, 91), 48.656923128349476, 5)))
	callback(REMOVE((95, 80), QUERY((51, 57), 45.13039243795071, 2)))
	callback(ADD((51, 67), QUERY((11, 63), 29.435920892711138, 1)))
	callback(ADD((16, 86), QUERY((73, 37), 15.318716103260641, 2)))
	callback(ADD((22, 93), QUERY((59, 62), 46.63702805662429, 8)))
	callback(ADD((82, 69), QUERY((86, 25), 6.30670748054273, 4)))
	callback(ADD((34, 91), QUERY((75, 18), 34.239559512171425, 8)))
	callback(REMOVE((34, 91), QUERY((82, 91), 16.562398155702596, 4)))
	callback(REMOVE((82, 69), QUERY((65, 15), 7.21921505097451, 1)))
	callback(REMOVE((16, 86), QUERY((83, 29), 25.384885563250236, 1)))
	callback(REMOVE((22, 93), QUERY((45, 36), 25.078340966445953, 4)))
	callback(REMOVE((51, 67), QUERY((75, 73), 7.51579214958209, 1)))
