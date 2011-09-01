from mtree.tests.fixtures.generator import ADD, REMOVE, QUERY
"""
actions = '11a11r11a11r'
dimensions = 1
remove_chance = 0.1
"""

DIMENSIONS = 1

def PERFORM(callback):
	callback(ADD((44,), QUERY((22,), 19.38985089849546, 3)))
	callback(ADD((96,), QUERY((69,), 69.40431459622837, 7)))
	callback(ADD((8,), QUERY((27,), 74.88277251888364, 4)))
	callback(ADD((32,), QUERY((9,), 30.45542310649349, 3)))
	callback(ADD((20,), QUERY((62,), 33.99819854934384, 3)))
	callback(ADD((64,), QUERY((85,), 70.43434371222301, 6)))
	callback(ADD((67,), QUERY((61,), 4.623160942819666, 11)))
	callback(ADD((82,), QUERY((27,), 29.779709860422283, 12)))
	callback(ADD((19,), QUERY((72,), 32.197788633474495, 12)))
	callback(ADD((15,), QUERY((36,), 15.309943604593705, 11)))
	callback(ADD((65,), QUERY((95,), 36.10761151157042, 17)))
	callback(REMOVE((20,), QUERY((54,), 56.0763258765136, 6)))
	callback(REMOVE((64,), QUERY((89,), 40.54924781263176, 2)))
	callback(REMOVE((44,), QUERY((77,), 52.8936140130559, 11)))
	callback(REMOVE((15,), QUERY((95,), 51.1268486854048, 11)))
	callback(REMOVE((32,), QUERY((62,), 64.1395878445445, 10)))
	callback(REMOVE((19,), QUERY((73,), 61.92009620082672, 8)))
	callback(REMOVE((8,), QUERY((24,), 3.9813744704573573, 1)))
	callback(REMOVE((67,), QUERY((10,), 72.23744459244318, 2)))
	callback(REMOVE((65,), QUERY((49,), 70.65660679851298, 2)))
	callback(REMOVE((82,), QUERY((39,), 0.3552525499918957, 3)))
	callback(REMOVE((96,), QUERY((84,), 9.446421930124611, 2)))
	callback(ADD((63,), QUERY((99,), 76.43845293039915, 4)))
	callback(ADD((2,), QUERY((44,), 28.29208518246361, 1)))
	callback(ADD((98,), QUERY((98,), 22.580775379595856, 3)))
	callback(ADD((74,), QUERY((73,), 37.67908513243428, 4)))
	callback(ADD((70,), QUERY((67,), 38.111175304452836, 4)))
	callback(ADD((3,), QUERY((86,), 36.148106332476615, 1)))
	callback(ADD((69,), QUERY((48,), 67.11556717780442, 8)))
	callback(ADD((92,), QUERY((24,), 44.09106221709412, 4)))
	callback(ADD((55,), QUERY((29,), 58.81100565248602, 10)))
	callback(ADD((79,), QUERY((83,), 38.097645619674225, 8)))
	callback(ADD((51,), QUERY((7,), 9.486117974846726, 11)))
	callback(REMOVE((3,), QUERY((62,), 71.26100758158313, 14)))
	callback(REMOVE((55,), QUERY((32,), 37.06189056358666, 5)))
	callback(REMOVE((69,), QUERY((46,), 57.19836995247717, 13)))
	callback(REMOVE((70,), QUERY((54,), 56.17733366733052, 1)))
	callback(REMOVE((51,), QUERY((40,), 31.383518520309046, 6)))
	callback(REMOVE((63,), QUERY((9,), 30.04860279301529, 6)))
	callback(REMOVE((79,), QUERY((66,), 41.25385075394824, 7)))
	callback(REMOVE((92,), QUERY((2,), 66.97875686991759, 4)))
	callback(REMOVE((98,), QUERY((59,), 24.27786312929988, 6)))
	callback(REMOVE((74,), QUERY((38,), 40.5028969392573, 3)))
	callback(REMOVE((2,), QUERY((60,), 69.37564320868023, 3)))
