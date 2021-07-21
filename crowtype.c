 /*
 * Copyright 2020-2021 Firecrow Silvernight
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
struct fcl_base_ifc;
struct fcl_string;

enum value_types {
    NO_VALUE = 0,
    FLOAT_VAL, 
    FUNCTION, 
    CELL,
    INT_VALUE,
    STRING,
    TRUE,
    FALSE
};

char *value_type_cstr[] = {
    "NO_VALUE",
    "FLOAT_VAL", 
    "FUNCTION", 
    "INT_VALUE",
    "CELL",
    "STRING",
    "TRUE",
    "FALSE",
};

enum argument_expectations {
    ARG = 0,
    KV_ARG = 1
};

typedef struct fcl_base_type {
    int id;
    int val_type;
    int flags;
    int ref_count;
    void (*destroy)(struct fcl_base_ifc *_s, enum value_types type);
} FclBaseType;

typedef struct fcl_base_ifc {
    FclBaseType base;
} FclBaseIfc;

/*
char *fcl_base_to_string(FclBaseType *t){
    return fcl_fmt("<FclBaseType type:%d ref_count:%d>", t->val_type, t->ref_count);
}
*/

void *_asType(void *_s, enum value_types t, char *file, int line, const char *func){
    FclBaseIfc *s = _s;
    if(!s || t != NO_VALUE && s->base.val_type != t){
        printf("asType: t and val_type mismatch at %s:%d:%s\n", file, line, func);
        return NULL;
    }
    return s;
}

void *checkType(void *_s, enum value_types t){
    FclBaseIfc *s = _s;
    if(!s || s->base.val_type != t){
        return NULL;
    }
    return s;
}

#define asType(ARG, TYPE) _asType((ARG), (TYPE), __FILE__,__LINE__,__func__)

void fcl_destroy(FclBaseIfc *_s, enum value_types t){
    FclBaseIfc *s;
    if(s = asType(_s, t)){
        free(s);
    }
}

typedef struct value {
    FclBaseType base;
} FclValue;

FclValue *fcl_value_alloc(int type){
    FclValue *v = malloc(sizeof(FclValue));
    bzero(v, sizeof(FclValue));
    v->base.val_type = type;
    v->base.destroy = fcl_destroy;
    return v;
}

int value_next_id = 1;

typedef struct fcl_int_val {
    FclBaseType base;
    int int_val; 
} FclInt;

FclInt *fcl_int_alloc(int n){
    FclInt *s = malloc(sizeof(FclInt));
    bzero(s, sizeof(FclInt));
    s->base.destroy = fcl_destroy;
	s->base.val_type = INT_VALUE;
    s->int_val = n;
    return s;
}

typedef struct fcl_string {
    FclBaseType base;
    struct crr *chars; 
} FclString;

void fcl_string_destroy(FclBaseIfc *_s, enum value_types _t){
    FclString *s;
    if(s = asType(_s, STRING)){
        crr_free(s->chars);
        free(s);
    }
}

FclString *fcl_string_alloc(char *cstr){
    FclString *s = malloc(sizeof(FclString));
    bzero(s, sizeof(FclString));
    s->base.val_type = STRING;
    s->base.destroy = fcl_string_destroy;
    s->chars = crr_alloc(); 
    if(cstr != NULL){
        crr_push(s->chars, cstr, strlen(cstr));
    }
    return s;
}

/* returns 1, 0*/
int GetLt(int gtlt){
    return gtlt > 0 ? 1 : 0;
}

int FclCmp(void *_a, void *_b){
    FclValue *a = _a;
    FclValue *b = _b;
    if(a->base.val_type != b->base.val_type){
        return -1; 
    };
    if(a->base.val_type == INT_VALUE){
        FclInt *na = asType((FclBaseIfc *)a, INT_VALUE);
        FclInt *nb = asType((FclBaseIfc *)b, INT_VALUE);
        if(na  && nb){
            return (na->int_val - nb->int_val);
        }
    }
    if(a->base.val_type == STRING){
        FclString *sa = asType(a, STRING);
        FclString *sb = asType(b, STRING);
        if(sa && sb){
            return crr_cmp(sa->chars,sb->chars);
        }
    }
    if(a->base.val_type == CELL){
        if(a->base.val_type == a->base.val_type) return 0;
        if(a->base.val_type == TRUE) return 1;
        return -1;
    }
    return -1;
}

int FclCmpCInt(void *a, int b){
    FclInt *n = asType(a, INT_VALUE);
    if(!n){
        return 1;
    }
    return n->int_val - b;
}

int FclStartsWithCStr(void *a, char *b, int len){
    FclString *r = asType(a, STRING);
    return strncmp(r->chars->data, b, len);
}

int FclCmpCStr(void *a, char *b){
    FclString *r = asType(a, STRING);
    return strncmp(r->chars->data, b, max(r->chars->length, strlen(b)));
}
