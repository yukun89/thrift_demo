namespace cpp SVR_NS

struct Data {
    1: required double x,
    2: required double y,
}

service SVR {
    Data add(1: Data v1, 2: Data v2),
    Data subtract(1: Data v1, 2: Data v2),
}

