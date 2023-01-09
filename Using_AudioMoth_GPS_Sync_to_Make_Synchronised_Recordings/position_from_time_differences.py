import itertools
import numpy as np
from scipy.optimize import least_squares

velocity_of_sound = 343.0

audiomoth_positions = np.array([[ 0,  3],
                                [ 3,  0],
                                [ 0, -3],
                                [-3,  0]])

time_differences = np.array([
                             [[   0, -406, -932, -594],
                              [   0,    0, -526, -193],
                              [   0,    0,    0,  333],
                              [   0,    0,    0,    0]],

                             [[   0, -406, -932, -589],
                              [   0,    0, -526, -188],
                              [   0,    0,    0,  339],
                              [   0,    0,    0,    0]]
                            ])

assert np.shape(audiomoth_positions)[0] == np.shape(time_differences)[1]

number_of_sensors = np.shape(audiomoth_positions)[0]

number_of_events = np.shape(time_differences)[0]

number_of_dimensions = np.shape(audiomoth_positions)[1]

initial_guess = np.zeros(number_of_dimensions)

position_estimates = np.zeros((number_of_events, number_of_dimensions))

pairs = list(itertools.combinations(range(number_of_sensors), 2))

p_ref = audiomoth_positions[[pair[0] for pair in pairs]]

p_comp = audiomoth_positions[[pair[1] for pair in pairs]]

for i in range(number_of_events):

    t_diff = np.array([time_differences[i][pair[0]][pair[1]] for pair in pairs])

    def eval_solution(x):
        return np.linalg.norm(x - p_ref, axis=1) \
                - np.linalg.norm(x - p_comp, axis=1) \
                + velocity_of_sound * t_diff / 1e6

    result = least_squares(eval_solution, initial_guess, method='lm')

    position_estimates[i] = result.x

print(position_estimates)

