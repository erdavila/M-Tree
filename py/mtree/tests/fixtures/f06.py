from mtree.tests.fixtures.generator import ADD, REMOVE, QUERY
"""
actions = '6a6r6a6r'
dimensions = 3
remove_chance = 0.1
"""

DIMENSIONS = 3

def PERFORM(callback):
	callback(ADD((31, 8, 78), QUERY((95, 72, 2), 51.96463862268654, 6)))
	callback(ADD((93, 84, 96), QUERY((34, 38, 54), 34.408104708781195, 1)))
	callback(ADD((85, 51, 93), QUERY((0, 83, 78), 3.7059358517378804, 4)))
	callback(ADD((83, 58, 77), QUERY((79, 29, 3), 5.73460301434908, 8)))
	callback(ADD((84, 73, 11), QUERY((89, 34, 29), 13.635968090643411, 2)))
	callback(ADD((18, 2, 32), QUERY((72, 52, 51), 31.27822678875451, 5)))
	callback(REMOVE((31, 8, 78), QUERY((43, 49, 43), 29.16964822395525, 1)))
	callback(REMOVE((85, 51, 93), QUERY((85, 84, 0), 16.58872196266236, 1)))
	callback(REMOVE((84, 73, 11), QUERY((29, 44, 29), 12.257462981374196, 4)))
	callback(REMOVE((18, 2, 32), QUERY((16, 90, 18), 44.58073074753799, 4)))
	callback(REMOVE((93, 84, 96), QUERY((15, 40, 88), 9.377545233317335, 6)))
	callback(REMOVE((83, 58, 77), QUERY((45, 81, 39), 75.87004149449061, 3)))
	callback(ADD((49, 94, 39), QUERY((39, 37, 30), 12.648377750008244, 5)))
	callback(ADD((41, 99, 34), QUERY((98, 62, 24), 69.7243028761528, 3)))
	callback(ADD((98, 79, 97), QUERY((100, 48, 97), 0.5500818804225016, 6)))
	callback(ADD((81, 90, 63), QUERY((25, 17, 25), 30.356434718133542, 9)))
	callback(ADD((15, 14, 33), QUERY((73, 87, 90), 31.16684798231727, 1)))
	callback(ADD((48, 72, 31), QUERY((3, 86, 63), 21.791659990082266, 8)))
	callback(REMOVE((41, 99, 34), QUERY((42, 23, 3), 52.031158226729595, 9)))
	callback(REMOVE((98, 79, 97), QUERY((17, 59, 99), 70.99708883712798, 4)))
	callback(REMOVE((49, 94, 39), QUERY((19, 83, 39), 32.042705943204695, 5)))
	callback(REMOVE((48, 72, 31), QUERY((13, 5, 66), 31.701627477076464, 7)))
	callback(REMOVE((81, 90, 63), QUERY((44, 39, 46), 7.380067113736395, 5)))
	callback(REMOVE((15, 14, 33), QUERY((70, 55, 19), 38.95255636736192, 2)))
