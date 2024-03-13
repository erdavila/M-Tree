from .generator import ADD, REMOVE, QUERY

"""
actions = '8a8r8a8r'
dimensions = 3
remove_chance = 0.1
"""

DIMENSIONS = 3


def PERFORM(callback):
    callback(ADD((62, 30, 40), QUERY((55, 48, 36), 79.92943142917025, 4)))
    callback(ADD((24, 83, 5), QUERY((87, 71, 79), 29.93522232091574, 4)))
    callback(ADD((61, 15, 8), QUERY((42, 99, 65), 54.44872373652812, 2)))
    callback(ADD((96, 22, 95), QUERY((42, 88, 24), 5.93585254386511, 1)))
    callback(ADD((91, 69, 15), QUERY((52, 87, 32), 10.238173894258988, 10)))
    callback(ADD((10, 8, 43), QUERY((88, 83, 34), 31.19527806475994, 6)))
    callback(ADD((87, 6, 92), QUERY((23, 29, 45), 29.559291300974692, 6)))
    callback(ADD((71, 91, 66), QUERY((32, 41, 82), 9.846272340032192, 11)))
    callback(REMOVE((24, 83, 5), QUERY((38, 16, 22), 18.72360616054155, 7)))
    callback(REMOVE((91, 69, 15), QUERY((24, 69, 98), 56.31958410590006, 11)))
    callback(REMOVE((62, 30, 40), QUERY((4, 20, 30), 72.97164983749147, 9)))
    callback(REMOVE((10, 8, 43), QUERY((85, 83, 5), 72.37691825097981, 2)))
    callback(REMOVE((96, 22, 95), QUERY((67, 50, 29), 46.24318160795637, 6)))
    callback(REMOVE((71, 91, 66), QUERY((52, 53, 8), 16.90341372515979, 5)))
    callback(REMOVE((61, 15, 8), QUERY((93, 97, 37), 27.44427177429842, 4)))
    callback(REMOVE((87, 6, 92), QUERY((25, 18, 19), 45.82484579032335, 1)))
    callback(ADD((50, 63, 12), QUERY((22, 53, 20), 33.758148232913676, 5)))
    callback(ADD((64, 62, 31), QUERY((3, 0, 50), 60.103676475268784, 2)))
    callback(ADD((95, 39, 37), QUERY((0, 30, 43), 29.808266104004744, 1)))
    callback(ADD((5, 97, 8), QUERY((63, 88, 74), 38.11713367406657, 1)))
    callback(ADD((73, 53, 50), QUERY((6, 63, 6), 39.66179803807896, 3)))
    callback(ADD((57, 20, 21), QUERY((89, 54, 87), 72.1435914914816, 5)))
    callback(ADD((91, 65, 0), QUERY((73, 70, 52), 32.9992222644089, 11)))
    callback(ADD((16, 16, 76), QUERY((8, 20, 61), 24.11119013648415, 2)))
    callback(REMOVE((5, 97, 8), QUERY((39, 65, 8), 14.374313876151135, 12)))
    callback(REMOVE((95, 39, 37), QUERY((88, 96, 33), 23.02947264004157, 7)))
    callback(REMOVE((16, 16, 76), QUERY((8, 59, 10), 47.25400596689401, 5)))
    callback(REMOVE((73, 53, 50), QUERY((85, 75, 4), 12.472803986094583, 7)))
    callback(REMOVE((91, 65, 0), QUERY((0, 57, 100), 67.89327690903575, 7)))
    callback(REMOVE((57, 20, 21), QUERY((33, 20, 61), 23.04127862795326, 7)))
    callback(REMOVE((50, 63, 12), QUERY((21, 25, 75), 35.75491178153249, 2)))
    callback(REMOVE((64, 62, 31), QUERY((35, 75, 53), 33.187451043263295, 1)))
