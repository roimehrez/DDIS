import pickle

import pyflann

def main():
    points = pickle.load(open('uint8_points.pkl'))
    #points = pickle.load(open('float32_points.pkl'))
    index = pyflann.FLANN()

    autotune_results = \
        index.build_index(points, algorithm='autotuned',
                                    target_precision=0.9,
                                    build_weight=0,
                                    memory_weight=0,
                                    sample_fraction=0.2,
                                    log_level='info')



if __name__ == '__main__':
    main()
