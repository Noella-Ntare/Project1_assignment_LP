/*
 * fastststats - CPython C extension for fast statistical computations.
 * Implements mean, variance, std, min, max over Python lists of floats.
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>

/* -----------------------------------------------------------------------
 * Internal helper: iterate a Python sequence and extract doubles into
 * a heap-allocated C array. Returns NULL on error (sets Python exception).
 * Caller must free() the returned pointer.
 * ----------------------------------------------------------------------- */
static double *
seq_to_doubles(PyObject *seq, Py_ssize_t *out_len)
{
    Py_ssize_t n = PySequence_Size(seq);
    if (n < 0) return NULL;
    if (n == 0) {
        PyErr_SetString(PyExc_ValueError, "sequence must be non-empty");
        return NULL;
    }

    double *arr = (double *)malloc(sizeof(double) * n);
    if (!arr) {
        PyErr_NoMemory();
        return NULL;
    }

    for (Py_ssize_t i = 0; i < n; i++) {
        PyObject *item = PySequence_GetItem(seq, i);
        if (!item) { free(arr); return NULL; }
        arr[i] = PyFloat_AsDouble(item);
        Py_DECREF(item);
        if (arr[i] == -1.0 && PyErr_Occurred()) {
            free(arr);
            return NULL;
        }
    }
    *out_len = n;
    return arr;
}

/* -----------------------------------------------------------------------
 * faststats.compute_mean(data) -> float
 * ----------------------------------------------------------------------- */
static PyObject *
faststats_mean(PyObject *self, PyObject *args)
{
    PyObject *seq;
    if (!PyArg_ParseTuple(args, "O", &seq)) return NULL;

    Py_ssize_t n;
    double *arr = seq_to_doubles(seq, &n);
    if (!arr) return NULL;

    double sum = 0.0;
    for (Py_ssize_t i = 0; i < n; i++) sum += arr[i];
    free(arr);

    return PyFloat_FromDouble(sum / (double)n);
}

/* -----------------------------------------------------------------------
 * faststats.compute_std(data) -> float
 * ----------------------------------------------------------------------- */
static PyObject *
faststats_std(PyObject *self, PyObject *args)
{
    PyObject *seq;
    if (!PyArg_ParseTuple(args, "O", &seq)) return NULL;

    Py_ssize_t n;
    double *arr = seq_to_doubles(seq, &n);
    if (!arr) return NULL;

    /* Single-pass Welford online mean/variance */
    double mean = 0.0, M2 = 0.0;
    for (Py_ssize_t i = 0; i < n; i++) {
        double delta = arr[i] - mean;
        mean += delta / (double)(i + 1);
        double delta2 = arr[i] - mean;
        M2 += delta * delta2;
    }
    free(arr);
    double variance = M2 / (double)n;   /* population std */
    return PyFloat_FromDouble(sqrt(variance));
}

/* -----------------------------------------------------------------------
 * faststats.full_stats(data) -> (mean, std, min, max)
 * ----------------------------------------------------------------------- */
static PyObject *
faststats_full_stats(PyObject *self, PyObject *args)
{
    PyObject *seq;
    if (!PyArg_ParseTuple(args, "O", &seq)) return NULL;

    Py_ssize_t n;
    double *arr = seq_to_doubles(seq, &n);
    if (!arr) return NULL;

    double mean = 0.0, M2 = 0.0;
    double mn = arr[0], mx = arr[0];

    for (Py_ssize_t i = 0; i < n; i++) {
        double x = arr[i];
        double delta = x - mean;
        mean += delta / (double)(i + 1);
        double delta2 = x - mean;
        M2 += delta * delta2;
        if (x < mn) mn = x;
        if (x > mx) mx = x;
    }
    free(arr);

    double std = sqrt(M2 / (double)n);
    return Py_BuildValue("(dddd)", mean, std, mn, mx);
}

/* -----------------------------------------------------------------------
 * Method table
 * ----------------------------------------------------------------------- */
static PyMethodDef FaststatsMethods[] = {
    {"compute_mean",  faststats_mean,       METH_VARARGS, "Compute arithmetic mean of a list of floats."},
    {"compute_std",   faststats_std,        METH_VARARGS, "Compute population standard deviation."},
    {"full_stats",    faststats_full_stats, METH_VARARGS, "Return (mean, std, min, max) for a list of floats."},
    {NULL, NULL, 0, NULL}
};

/* -----------------------------------------------------------------------
 * Module definition
 * ----------------------------------------------------------------------- */
static struct PyModuleDef faststatsmodule = {
    PyModuleDef_HEAD_INIT,
    "faststats",
    "C extension for fast statistical computations on numeric arrays.",
    -1,
    FaststatsMethods
};

PyMODINIT_FUNC
PyInit_faststats(void)
{
    return PyModule_Create(&faststatsmodule);
}
