struct CoordAux {
    int x;
    int y;
};

struct VectorDoubles {
    u_int V_value2_len;
    double V_value2_val[32];
};

struct Peticion {
    int key;
    string value1<256>;
    int N_value2;
    struct VectorDoubles V_value2;
    CoordAux value3;
};

struct Respuesta {
    int result;
};

struct RespuestaGetValue {
    int result; /* 0 para éxito, -1 para error */
    string value1<256>;
    int N_value2;
    struct VectorDoubles V_value2;
    CoordAux value3;
};

/* Definición del programa RPC */
program TUPLAS_PROG {
    version TUPLAS_VERS {
        Respuesta destroy_1(void) = 1;
        Respuesta set_value_1(Peticion) = 2;
        RespuestaGetValue get_value_1(Peticion) = 3;
        Respuesta modify_value_1(Peticion) = 4;
        Respuesta delete_key_1(Peticion) = 5;
        Respuesta exist_1(Peticion) = 6;

    } = 1;
} = 0x20000001;