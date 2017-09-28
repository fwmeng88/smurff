#!/usr/bin/env python3

from macau import macau
import matrix_io as mio
import scipy.io
import argparse

parser = argparse.ArgumentParser(description='Black SMURFF - a smurff-compatible commandline for Macau')

options = {
        "group1":         { "group": "Priors and side Info"},
        "row-prior":  	  { "metavar": "PRIOR", "default": "normal", "help": "One of <normal|spikeandslab|macau|macauone>"},
        "col-prior":  	  { "metavar": "PRIOR", "default": "normal", "help": "One of <normal|spikeandslab|macau|macauone>"},
        "row-features":   { "metavar": "FILE",  "help": "side info for rows"},
        "col-features":   { "metavar": "FILE",  "help": "side info for cols"},
        #unsupported "row-model":      { "metavar": "FILE",  "help": "initialization matrix for row model"},
        #unsupported "col-model":      { "metavar": "FILE",  "help": "initialization matrix for col model"},
        #unsupported "center":         { "metavar": "MODE",  "default": "none", "help": "center <global|rows|cols|none>"},
        "group2":         { "group": "Test and train matrices"},
        "test":  	  { "metavar": "FILE", "help":  "test data (for computing RMSE}"},
        "train":  	  { "metavar": "FILE", "help":  "train data file", "required": True},
        "group3":         { "group": "General parameters"},
        "burnin":  	  { "metavar": "NUM", "type": int,  "default": 200,  "help":  "200  number of samples to discard"},
        "nsamples":  	  { "metavar": "NUM", "type": int,   "default": 800, "help":  "800  number of samples to collect"},
        "num-latent":  	  { "metavar": "NUM", "type": int,   "default": 96, "help":  "96  number of latent dimensions"},
        #unsupported "restore-prefix": { "metavar": "PATH", "help":  "prefix for file to initialize stae"},
        #unsupported "restore-suffix": { "metavar": "EXT",  "help":  "suffix for initialization files (.csv or .ddm}"},
        #unsupported "init-model":     { "metavar": "NAME", "help":  "One of <random|zero>"},
        "save-prefix":    { "metavar": "PATH", "help":  "prefix for result files"},
        #unsupported "save-suffix":    { "metavar": "EXT",  "help":  "suffix for result files (.csv or .ddm}"},
        #unsupported "save-freq":      { "metavar": "NUM", "type": int,  "help":  "save every n iterations (0 == never}"},
        #unsupported "threshold":      { "metavar": "NUM", "type": float,  "help":  "threshold for binary classification"},
        "verbose":        { "metavar": "NUM", "nargs": "?", "type": int,  "help":  "verbose output (default = 1}"},
        "quiet":          {                    "help":  "no output"},
        #unsupported "status":         { "metavar": "FILE", "help":  "output progress to csv file"},
        "group4":         { "group": "Noise model" },
        "precision":  	  { "metavar": "NUM", "type": float, "help":  "precision of observations"},
        "adaptive":  	  { "metavar": "NUM,NUM", "nargs": "?", "help":  "1.0,10.0  adaptive precision of observations"},
        "group1":         { "group": "For the macau prior"},
        "lambda-beta":    { "metavar": "NUM", "type": float,  "default": 10.0, "help":  "10.0  initial value of lambda beta"},
        "tol":            { "metavar": "NUM", "type": float,   "default": 1e-6, "help":  "1e-6  tolerance for CG"},
        #unsupported "direct":         { "default": False, "help":  "Use Cholesky decomposition i.o. CG Solver"},
}

group = parser
for name, opt in options.items():
    if ("group" in opt):
        group = parser.add_argument_group(opt["group"])
    else:
        group.add_argument("--" + name, **opt)

args = parser.parse_args()

def read_matrix(fname):
    if (not fname):
        return None
    if fname.endswith('.mtx') or fname.endswith('.mm'):
        return scipy.io.mmread(fname)
    elif fname.endswith('.sbm'):
        return mio.read_sparse_binary_matrix(fname)
    elif fname.endswith('.sdm'):
        return mio.read_sparse_float64(fname)
    elif fname.endswith('.ddm'):
        return mio.read_dense_float64(fname)
    else:
        raise Exception("Unknown matrix type: " + fname)

 

train_matrix     = read_matrix(args.train)
test_matrix      = read_matrix(args.test)
side_rows_matrix = read_matrix(args.row_features)
side_cols_matrix = read_matrix(args.col_features)

# deduce noise model to use
if ("adaptive" in vars(args)):
    try:
        args.sn_max = float(args.adaptive.split(",")[1])
    except:
        args.sn_max = 10.0
    assert(not args.precision)
    args.precision = "adaptive"
else:
    args.sn_max = 10.0
    if (not args.precision):
        args.precision = 5.0

# check row- and col-prior
def check_prior(prior_name, side_info):
    if (prior_name == "normal"):
        assert(not side_info)
    elif (prior_name == "macau"):
        assert(side_info)
    elif (prior_name == "macauone"):
        assert(side_info)
    else:
        raise Exception("Unknown prior: " + prior_name)

check_prior(args.row_prior, args.row_features)
check_prior(args.col_prior, args.col_features)

args.univariate = False
if (args.row_prior == "macauone" or args.col_prior == "macauone"):
    args.univariate = True

if ("verbose" in vars(args) and not args.verbose):
    args.verbose = 1

if (args.verbose):
    from pprint import pprint
    print("Running black SMURFF with these options: ")
    pprint(vars(args), indent=2)

result = macau(Y           = train_matrix,
               Ytest       = test_matrix,
               side = [ side_rows_matrix, side_cols_matrix ],
               lambda_beta = args.lambda_beta,
               num_latent  = args.num_latent,
               precision   = args.precision,
               burnin      = args.burnin,
               nsamples    = args.nsamples,
               univariate  = args.univariate,
               tol         = args.tol,
               sn_max      = args.sn_max,
               verbose     = args.verbose,
               save_prefix = args.save_prefix)

if (args.verbose):
    print(result)

if (args.save_prefix):
    pred_fname = args.save_prefix + "-predictions.csv"
    if(args.verbose):
        print("Saving predictions in " + pred_fname)
    result.prediction.to_csv(pred_fname)
