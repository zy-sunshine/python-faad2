#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <neaacdec.h>

typedef struct {
    PyObject_HEAD
    NeAACDecHandle hAac;
} Faad2Dec;

static void
faad2_Faad2Dec_dealloc(Faad2Dec *self)
{
    NeAACDecClose(self->hAac);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int
faad2_Faad2Dec___init___impl(Faad2Dec *self)
{
    self->hAac = NeAACDecOpen();
    if (!self->hAac)
        goto error;
    return 0;

error:
    return -1;
}

static int
faad2_Faad2Dec___init__(PyObject *self, PyObject *args, PyObject *kwargs)
{
    int return_value = -1;

    //if ((Py_TYPE(self) == &Faad2Dec_Type) &&
    //    !_PyArg_NoPositional("Faad2Dec", args))
    //    goto exit;
    //if ((Py_TYPE(self) == &Faad2Dec_Type) &&
    //    !_PyArg_NoKeywords("Faad2Dec", kwargs))
    //    goto exit;
    return_value = faad2_Faad2Dec___init___impl((Faad2Dec *)self);

//exit:
    return return_value;
}

static PyObject *
Decode_impl(Faad2Dec *self, char *data, size_t len)
{
    NeAACDecFrameInfo hInfo;
    void * retbuf;

    retbuf = NeAACDecDecode(self->hAac, &hInfo, (unsigned char *)data, len);
    if ((hInfo.error == 0) && (hInfo.samples > 0)) {
        // do what you need to do with the decoded samples
        return PyBytes_FromStringAndSize(retbuf, hInfo.bytesconsumed);
    } else if (hInfo.error != 0) {
        // Some error occurred while decoding this frame
        PyErr_Format(PyExc_RuntimeError, "error occurred hInfo.error %d in '%s' object",
            hInfo.error, Py_TYPE(self)->tp_name);
        return NULL;
    } else {
        PyErr_Format(PyExc_RuntimeError, "unexpected end in '%s' object",
            Py_TYPE(self)->tp_name);
    }
    return NULL;
}

static PyObject *
Faad2Dec_Decode(Faad2Dec *self, PyObject *args, PyObject *kwargs)
{
    PyObject *return_value = NULL;
    static char *_keywords[] = {"data", NULL};
    Py_buffer data = {NULL, NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*:Decode", _keywords, &data))
        goto exit;
    return_value = Decode_impl(self, data.buf, data.len);

exit:
    /* Cleanup for data */
    if (data.obj)
       PyBuffer_Release(&data);

    return return_value;
}

static PyObject *
Faad2Dec_getstate(Faad2Dec *self, PyObject *noargs)
{
    PyErr_Format(PyExc_TypeError, "cannot serialize '%s' object",
                 Py_TYPE(self)->tp_name);
    return NULL;
}

static PyObject *
Faad2Dec_GetCapabilities(Faad2Dec *self, PyObject *noargs)
{
    return PyLong_FromLong(NeAACDecGetCapabilities());
}

static PyObject *
Faad2Dec_GetCurrentConfiguration(Faad2Dec *self, PyObject *noargs)
{
    //typedef struct NeAACDecConfiguration
    //{
    //    unsigned char defObjectType;
    //    unsigned long defSampleRate;
    //    unsigned char outputFormat;
    //    unsigned char downMatrix;
    //    unsigned char useOldADTSFormat;
    //} NeAACDecConfiguration, *NeAACDecConfigurationPtr;
    NeAACDecConfigurationPtr conf = NeAACDecGetCurrentConfiguration(self->hAac);
    return Py_BuildValue("{s:b,s:l,s:b,s:b,s:b}",
        "defObjectType", conf->defObjectType,
        "defSampleRate", conf->defSampleRate,
        "outputFormat", conf->outputFormat,
        "downMatrix", conf->downMatrix,
        "useOldADTSFormat", conf->useOldADTSFormat
    );
}

static PyObject *
Faad2Dec_SetConfiguration(Faad2Dec *self, PyObject *args, PyObject *kwargs)
{
    static char *_keywords[] = {
        "defObjectType",
        "defSampleRate",
        "outputFormat",
        "downMatrix",
        "useOldADTSFormat",
        NULL
    };
    NeAACDecConfiguration conf;


    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "bIbbb:SetConfiguration", _keywords,
            &conf.defObjectType, &conf.defSampleRate, &conf.outputFormat, &conf.downMatrix,
            &conf.useOldADTSFormat))
        goto error;
    if (0 == NeAACDecSetConfiguration(self->hAac, &conf)) {
        PyErr_Format(PyExc_RuntimeError, "error occurred NeAACDecSetConfiguration in '%s' object",
            Py_TYPE(self)->tp_name);
        goto error;
    }
    Py_RETURN_NONE;

error:
    return NULL;
}

/*
 * Initialises the decoder library based on an AudioSpecificConfig as found inside a
 * MP4 file.
 * This function fills the samplerate and channels parameters with the detected
 * values.
 */
static PyObject *
Faad2Dec_Init2(Faad2Dec *self, PyObject *args, PyObject *kwargs)
{
    unsigned long samplerate;
    unsigned char channels;

    Py_buffer data = {NULL, NULL};

    static char *_keywords[] = {
        "buffer",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*:Init2", _keywords, &data))
        goto error;
    if (0 == NeAACDecInit2(self->hAac, (unsigned char*)data.buf, (unsigned long)data.len,
        &samplerate, &channels)) {
        PyErr_Format(PyExc_RuntimeError, "error occurred NeAACDecInit2 in '%s' object",
            Py_TYPE(self)->tp_name);
        goto error;
    }

    return Py_BuildValue("{s:l,s:b}",
        "samplerate", samplerate,
        "channels", channels
    );

error:
    return NULL;
}

static PyObject *
Faad2Dec_Init(Faad2Dec *self, PyObject *args, PyObject *kwargs)
{
    unsigned long samplerate;
    unsigned char channels;

    Py_buffer data = {NULL, NULL};

    static char *_keywords[] = {
        "buffer",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|y*:Init2", _keywords, &data))
        goto error;
    if (0 == NeAACDecInit(self->hAac, (unsigned char*)data.buf, (unsigned long)data.len,
        &samplerate, &channels)) {
        PyErr_Format(PyExc_RuntimeError, "error occurred NeAACDecInit2 in '%s' object",
            Py_TYPE(self)->tp_name);
        goto error;
    }

    return Py_BuildValue("{s:l,s:b}",
        "samplerate", samplerate,
        "channels", channels
    );

error:
    return NULL;
}

static PyMethodDef Faad2Dec_methods[] = {
    {"GetCapabilities", (PyCFunction)Faad2Dec_GetCapabilities, METH_NOARGS},
    {"GetCurrentConfiguration", (PyCFunction)Faad2Dec_GetCurrentConfiguration, METH_NOARGS},
    {"SetConfiguration", (PyCFunction)Faad2Dec_SetConfiguration, METH_VARARGS|METH_KEYWORDS, NULL},
    {"Init2", (PyCFunction)Faad2Dec_Init2, METH_VARARGS|METH_KEYWORDS, NULL},
    {"Init", (PyCFunction)Faad2Dec_Init, METH_VARARGS|METH_KEYWORDS, NULL},
    {"Decode", (PyCFunction)Faad2Dec_Decode, METH_VARARGS|METH_KEYWORDS, NULL},
    {"__getstate__", (PyCFunction)Faad2Dec_getstate, METH_NOARGS},
    {NULL}
};

static PyMemberDef Faad2Dec_members[] = {
    {NULL}
};

static PyTypeObject Faad2Dec_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "faad2.Faad2Dec",                    /* tp_name */
    sizeof(Faad2Dec),                   /* tp_basicsize */
    0,                                  /* tp_itemsize */
    (destructor)faad2_Faad2Dec_dealloc,       /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,                                  /* tp_reserved */
    0,                                  /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    0,                                  /* tp_hash  */
    0,                                  /* tp_call */
    0,                                  /* tp_str */
    0,                                  /* tp_getattro */
    0,                                  /* tp_setattro */
    0,                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                 /* tp_flags */
    NULL,                               /* tp_doc */
    0,                                  /* tp_traverse */
    0,                                  /* tp_clear */
    0,                                  /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */
    Faad2Dec_methods,            /* tp_methods */
    Faad2Dec_members,            /* tp_members */
    0,                                  /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    faad2_Faad2Dec___init__,      /* tp_init */
    0,                                  /* tp_alloc */
    PyType_GenericNew,                  /* tp_new */
};

static PyObject *
faad2_shell(PyObject *self, PyObject *args)
{
    const char *command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command))
        return NULL;
    sts = system(command);
    return PyLong_FromLong(sts);
}

static PyMethodDef Faad2Methods[] = {
    {"shell",  faad2_shell, METH_VARARGS, "Execute a shell command."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef faad2module = {
    PyModuleDef_HEAD_INIT,
    "faad2",   /* name of module */
    NULL,      /* module documentation, may be NULL */
    -1,        /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    Faad2Methods
};

PyMODINIT_FUNC
PyInit_faad2(void)
{
    PyObject *m;

    if (PyType_Ready(&Faad2Dec_Type) < 0)
        return NULL;

    m = PyModule_Create(&faad2module);
    if (m == NULL)
        return NULL;

    Py_INCREF(&Faad2Dec_Type);
    PyModule_AddObject(m, "Faad2Dec", (PyObject *)&Faad2Dec_Type);

    return m;
}
