import _parca
from MatrixInfo import *
from _base import *
from _util import *

__all__ = [
    "set_score_matrix",
    "process_direct_stage",
    "process_backward_stage",
    "get_pareto_alignments",
    "get_primary_alignments",
    "alignment_to_string",
    "get_common_part",
    "set_temporary_directory",
    "get_last_error"
]

for mn in available_matrices:
    if mn.startswith("pam") or mn.startswith("blosum"):
        __all__ += [mn]

from _custom_matrices import *
__all__ += [ "pam240", "pam360", "pam480" ]

