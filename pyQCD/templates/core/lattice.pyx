cdef class {{ typedef.name }}:

    def __cinit__(self, shape, int site_size=1):
        self.lexico_layout = new LexicoLayout(shape)
        self.view_count = 0
        self.site_size = site_size
        self.instance = new _{{ typedef.cname }}(self.lexico_layout[0],  _{{ typedef.element_type.cname }}(_{{ typedef.element_type.cname }}_zeros()), site_size)

    def __dealloc__(self):
        del self.instance
        del self.lexico_layout

    def __getbuffer__(self, Py_buffer* buffer, int flags):
        cdef Py_ssize_t itemsize = sizeof(atomics.Complex)

        {% set buffer_iter, buffer_size = typedef.buffer_info("itemsize") %}
        {% for shape, stride in buffer_iter %}
        self.buffer_shape[{{ loop.index0 }}] = {{ stride }}
        self.buffer_strides[{{ loop.index0 }}] = {{ shape }}
        {% endfor %}

        buffer.buf = <char*>&(self.instance[0][0])

        {% set num_format = "dd" if precision == "double" else "ff" %}
        buffer.format = "{{ num_format }}"
        buffer.internal = NULL
        buffer.itemsize = itemsize
        buffer.len = {{ buffer_size }}
        buffer.ndim = {{ typedef.buffer_ndims }}

        buffer.obj = self
        buffer.readonly = 0
        buffer.shape = self.buffer_shape
        buffer.strides = self.buffer_strides
        buffer.suboffsets = NULL

        self.view_count += 1

    def __releasebuffer__(self, Py_buffer* buffer):
        self.view_count -= 1

    property as_numpy:
        """Return a view to this object as a numpy array"""
        def __get__(self):
            out = np.asarray(self)
            out.dtype = complex
            return out.reshape({{ typedef.shape_expr }})

        def __set__(self, value):
            out = np.asarray(self)
            out.dtype = complex
            out = out.reshape({{ typedef.shape_expr }})
            out[:] = value