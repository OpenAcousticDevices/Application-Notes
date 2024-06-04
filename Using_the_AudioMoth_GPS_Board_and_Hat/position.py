import numpy as np

from scipy.optimize import least_squares

velocity_of_sound = 343.0

audiomoth_positions = np.array([[ 55.47, -29.86],
                                [ 55.16,  29.98],
                                [-54.76, -29.82],
                                [-55.04,  29.72]])

time_of_arrival = np.array([[ 11.3823, 11.3797, 11.3808, 11.3804],
                            [ 71.1305, 71.1281, 71.1294, 71.1303]])

assert np.shape(audiomoth_positions)[0] == np.shape(time_of_arrival)[1]

number_of_events = np.shape(time_of_arrival)[0]

number_of_dimensions = np.shape(audiomoth_positions)[1]

initial_guess = np.zeros(number_of_dimensions)

position_estimates = np.zeros((number_of_events, number_of_dimensions))

for i in range(number_of_events):

    c = np.argmin(time_of_arrival[i])

    t_c = time_of_arrival[i][c]
    p_c = np.expand_dims(audiomoth_positions[c], axis=0)

    all_t_i = np.delete(time_of_arrival[i], c, axis=0)
    all_p_i = np.delete(audiomoth_positions, c, axis=0)

    def eval_solution(x):
        return np.linalg.norm(x - p_c, axis=1) \
                - np.linalg.norm(x - all_p_i, axis=1) \
                + velocity_of_sound * (all_t_i - t_c) 

    result = least_squares(eval_solution, initial_guess, method='lm')

    position_estimates[i] = result.x

print(position_estimates)