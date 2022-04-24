import argparse
import subprocess
import os
import copy

TREATMENTS_BASE = [
    'cmoea',
    'singlebin',
    'nsga',
    'lexicase',
    'elexicase_pmad_pmax',
    'nsga3',
    'combt_nsga',
    'combt_lexicase',
    'combt_elexicase_pmad_pmax',
    'combt_nsga3',
]
EXEC_FILE_STR = 'hardeasy_rls_{treatment}_hg={hard_genes}_eg={easy_genes}_sg={shared_genes}{options}'
OPTIONS_STR = 'RLS {treatment} HG={hard_genes} EG={easy_genes} SG={shared_genes}{options}'
FIT_FUNC = {
    "mean": ("mean_func", ""),
    "rast": ("mod_rast_func", ""),
}
GENERATIONS = {
    500: ("gen500", ""),
    1000: ("gen1000", "GEN1000"),
    2500: ("gen2500", "GEN2500"),
    5000: ("gen5000", "GEN5000"),
}
POPULATION = {
    120: ("pop120", "POP120"),
    240: ("pop240", "POP240"),
    300: ("pop300", "POP300"),
}
CROSSOVER = {
    True: ("cross", ""),
    False: ("nocross", "NOCROSS"),
}
DEFINED_STRINGS = ['#if defined(', '#elif defined(', '#ifndef ']


class PreventDuplicates(argparse.Action):
    def __init__(self, option_strings, dest, **kwargs):
        super().__init__(option_strings, dest, **kwargs)
        self.occurrence_count = 0

    def __call__(self, parser, namespace, values, option_string=None):
        self.occurrence_count += 1
        if self.occurrence_count > 1:
            raise argparse.ArgumentError(self, 'Argument can only be provided once.')
        setattr(namespace, self.dest, values)


def check_options(options: str):
    defined_options = set()
    with open('exp/cmoea/hard_easy_function.cpp', 'r') as fh:
        for line in fh:
            for defined_str in DEFINED_STRINGS:
                if line.startswith(defined_str):
                    start = len(defined_str)
                    end = line.find(')')
                    defined_options.add(line[start:end])
    # print('defined_options:', defined_options)

    options_set = [opt.split('=')[0] for opt in options.split()]
    # print("options_set:", options_set)
    for option in options_set:
        assert option in defined_options, f'option {option} not found in {defined_options}'


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-t', dest='treatments', type=str, nargs='*', default=TREATMENTS_BASE, action=PreventDuplicates)
    parser.add_argument('-o', dest='options', type=str, nargs='*', default=[], action=PreventDuplicates)
    parser.add_argument('--fit', dest='fit', type=str, default='rast', action=PreventDuplicates)
    parser.add_argument('--gen', dest='gen', type=int, default=500, action=PreventDuplicates)
    parser.add_argument('--pop', dest='pop', type=int, default=120, action=PreventDuplicates)
    parser.add_argument('--hg', dest='hg', type=int, default=20, action=PreventDuplicates)
    parser.add_argument('--eg', dest='eg', type=int, default=10, action=PreventDuplicates)
    parser.add_argument('--sg', dest='sg', type=int, default=10, action=PreventDuplicates)
    parser.add_argument('-r', dest='runs', type=int, default=20, action=PreventDuplicates)
    parser.add_argument('--nocross', dest='nocross', action='store_true', default=False)
    parser.add_argument('-d,--dry_run', dest='dry_run', action='store_true', default=False)
    args = parser.parse_args()

    options = copy.copy(args.options)
    dir_name = ''
    name_ext, option = FIT_FUNC[args.fit]
    dir_name += name_ext
    if option != "":
        options.append(option)
    name_ext, option = GENERATIONS[args.gen]
    dir_name += '_' + name_ext
    if option != "":
        options.append(option)
    name_ext, option = CROSSOVER[not args.nocross]
    dir_name += '_' + name_ext
    if option != "":
        options.append(option)
    name_ext, option = POPULATION[args.pop]
    dir_name += '_' + name_ext
    if option != "":
        options.append(option)

    dir_name += f'_hg_{args.hg}_eg_{args.eg}_sg_{args.sg}'

    if len(options) > 0:
        options_str = '_' + '_'.join(options)
    else:
        options_str = ''

    if len(args.options) > 0:
        dir_name += '_' + '_'.join(args.options).lower()

    print("args.options:", args.options)
    print("treatments:", args.treatments)
    print("dir_name:", dir_name)
    root_dir = os.getcwd()

    for treatment in args.treatments:
        treatment_file = EXEC_FILE_STR.format(treatment=treatment,
                                              hard_genes=args.hg,
                                              easy_genes=args.eg,
                                              shared_genes=args.sg,
                                              options=options_str)
        treatment_file = treatment_file.lower()
        print("Executable to run:", treatment_file)

        env_var_value = OPTIONS_STR.format(treatment=treatment,
                                           hard_genes=args.hg,
                                           easy_genes=args.eg,
                                           shared_genes=args.sg,
                                           options=options_str)
        env_var_value = env_var_value.upper().replace('_', ' ')
        print("Options:", env_var_value)
        check_options(env_var_value)
        os.environ['SFERES_VARIANT'] = env_var_value
        treatment_dir = os.path.join(root_dir, dir_name, treatment)
        print("Treatment directory:", treatment_dir)
        executable = os.path.join(root_dir, 'build/exp/cmoea', treatment_file)
        print("Executable:", executable)

        if not args.dry_run:
            subprocess.run(['./waf', '--exp=cmoea'], cwd=root_dir)
            os.makedirs(treatment_dir, exist_ok=True)

        print("args.runs:", args.runs)
        for i in range(args.runs):
            if not args.dry_run:
                subprocess.run([executable], cwd=treatment_dir)


if __name__ == '__main__':
    main()
