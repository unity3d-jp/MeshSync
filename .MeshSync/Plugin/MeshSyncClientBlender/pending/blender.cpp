#include "pch.h"


void BKE_object_rot_to_mat3(Object *ob, float mat[3][3], bool use_drot)
{
    float rmat[3][3], dmat[3][3];

    /* 'dmat' is the delta-rotation matrix, which will get (pre)multiplied
    * with the rotation matrix to yield the appropriate rotation
    */

    /* rotations may either be quats, eulers (with various rotation orders), or axis-angle */
    if (ob->rotmode > 0) {
        /* euler rotations (will cause gimble lock, but this can be alleviated a bit with rotation orders) */
        eulO_to_mat3(rmat, ob->rot, ob->rotmode);
        eulO_to_mat3(dmat, ob->drot, ob->rotmode);
    }
    else if (ob->rotmode == ROT_MODE_AXISANGLE) {
        /* axis-angle - not really that great for 3D-changing orientations */
        axis_angle_to_mat3(rmat, ob->rotAxis, ob->rotAngle);
        axis_angle_to_mat3(dmat, ob->drotAxis, ob->drotAngle);
    }
    else {
        /* quats are normalized before use to eliminate scaling issues */
        float tquat[4];

        normalize_qt_qt(tquat, ob->quat);
        quat_to_mat3(rmat, tquat);

        normalize_qt_qt(tquat, ob->dquat);
        quat_to_mat3(dmat, tquat);
    }

    /* combine these rotations */
    if (use_drot)
        mul_m3_m3m3(mat, dmat, rmat);
    else
        copy_m3_m3(mat, rmat);
}


#ifdef USE_DATA_PTR
#  define CHUNK_DATA(chunk) (chunk)->_data
#else
#  define CHUNK_DATA(chunk) (CHECK_TYPE_INLINE(chunk, BLI_mempool_chunk *), (void *)((chunk) + 1))
#endif

/* Little Endian */
#  define MAKE_ID(a, b, c, d) ( (int)(d) << 24 | (int)(c) << 16 | (b) << 8 | (a) )
#  define MAKE_ID_8(a, b, c, d, e, f, g, h) \
	((int64_t)(h) << 56 | (int64_t)(g) << 48 | (int64_t)(f) << 40 | (int64_t)(e) << 32 | \
	 (int64_t)(d) << 24 | (int64_t)(c) << 16 | (int64_t)(b) <<  8 | (a) )

#define FREEWORD ((sizeof(void *) > sizeof(int32_t)) ? \
	MAKE_ID_8('e', 'e', 'r', 'f', 'f', 'r', 'e', 'e') : \
	  MAKE_ID('e',           'f', 'f',           'e'))

typedef struct BLI_freenode {
    struct BLI_freenode *next;
    intptr_t freeword; /* used to identify this as a freed node */
} BLI_freenode;

struct BLI_mempool {
    BLI_mempool_chunk *chunks;  /* single linked list of allocated chunks */
                                /* keep a pointer to the last, so we can append new chunks there
                                * this is needed for iteration so we can loop over chunks in the order added */
    BLI_mempool_chunk *chunk_tail;

    unsigned int esize;         /* element size in bytes */
    unsigned int csize;         /* chunk size in bytes */
    unsigned int pchunk;        /* number of elements per chunk */
    unsigned int flag;
    /* keeps aligned to 16 bits */

    BLI_freenode *free;         /* free element list. Interleaved into chunk datas. */
    unsigned int maxchunks;     /* use to know how many chunks to keep for BLI_mempool_clear */
    unsigned int totused;       /* number of elements currently in use */
#ifdef USE_TOTALLOC
    unsigned int totalloc;          /* number of elements allocated in total */
#endif
};

typedef struct BLI_mempool_chunk {
    struct BLI_mempool_chunk *next;
#ifdef USE_DATA_PTR
    void *_data;
#endif
} BLI_mempool_chunk;


void BLI_mempool_iternew(BLI_mempool *pool, BLI_mempool_iter *iter)
{
    BLI_assert(pool->flag & BLI_MEMPOOL_ALLOW_ITER);

    iter->pool = pool;
    iter->curchunk = pool->chunks;
    iter->curindex = 0;
}
void *BLI_mempool_iterstep(BLI_mempool_iter *iter)
{
    if (UNLIKELY(iter->curchunk == NULL)) {
        return NULL;
    }

    const unsigned int esize = iter->pool->esize;
    BLI_freenode *curnode = (BLI_freenode*)POINTER_OFFSET(CHUNK_DATA(iter->curchunk), (esize * iter->curindex));
    BLI_freenode *ret;
    do {
        ret = curnode;

        if (++iter->curindex != iter->pool->pchunk) {
            curnode = (BLI_freenode*)POINTER_OFFSET(curnode, esize);
        }
        else {
            iter->curindex = 0;
            iter->curchunk = iter->curchunk->next;
            if (iter->curchunk == NULL) {
                return (ret->freeword == FREEWORD) ? NULL : ret;
            }
            curnode = (BLI_freenode*)CHUNK_DATA(iter->curchunk);
        }
    } while (ret->freeword == FREEWORD);

    return ret;
}



void bmiter__elem_of_mesh_begin(struct BMIter__elem_of_mesh *iter)
{
#ifdef USE_IMMUTABLE_ASSERT
    ((BMIter *)iter)->count = BLI_mempool_count(iter->pooliter.pool);
#endif
    BLI_mempool_iternew(iter->pooliter.pool, &iter->pooliter);
}
void *bmiter__elem_of_mesh_step(struct BMIter__elem_of_mesh *iter)
{
#ifdef USE_IMMUTABLE_ASSERT
    BLI_assert(((BMIter *)iter)->count <= BLI_mempool_count(iter->pooliter.pool));
#endif
    return BLI_mempool_iterstep(&iter->pooliter);
}

void  bmiter__edge_of_vert_begin(struct BMIter__edge_of_vert *iter)
{
    if (iter->vdata->e) {
        iter->e_first = iter->vdata->e;
        iter->e_next = iter->vdata->e;
    }
    else {
        iter->e_first = NULL;
        iter->e_next = NULL;
    }
}
void  *bmiter__edge_of_vert_step(struct BMIter__edge_of_vert *iter)
{
    BMEdge *e_curr = iter->e_next;

    if (iter->e_next) {
        iter->e_next = bmesh_disk_edge_next(iter->e_next, iter->vdata);
        if (iter->e_next == iter->e_first) {
            iter->e_next = NULL;
        }
    }

    return e_curr;
}

void  bmiter__face_of_vert_begin(struct BMIter__face_of_vert *iter)
{
    ((BMIter *)iter)->count = bmesh_disk_facevert_count(iter->vdata);
    if (((BMIter *)iter)->count) {
        iter->l_first = bmesh_disk_faceloop_find_first(iter->vdata->e, iter->vdata);
        iter->e_first = iter->l_first->e;
        iter->e_next = iter->e_first;
        iter->l_next = iter->l_first;
    }
    else {
        iter->l_first = iter->l_next = NULL;
        iter->e_first = iter->e_next = NULL;
    }
}
void  *bmiter__face_of_vert_step(struct BMIter__face_of_vert *iter)
{
    BMLoop *l_curr = iter->l_next;

    if (((BMIter *)iter)->count && iter->l_next) {
        ((BMIter *)iter)->count--;
        iter->l_next = bmesh_radial_faceloop_find_next(iter->l_next, iter->vdata);
        if (iter->l_next == iter->l_first) {
            iter->e_next = bmesh_disk_faceedge_find_next(iter->e_next, iter->vdata);
            iter->l_first = bmesh_radial_faceloop_find_first(iter->e_next->l, iter->vdata);
            iter->l_next = iter->l_first;
        }
    }

    if (!((BMIter *)iter)->count) {
        iter->l_next = NULL;
    }

    return l_curr ? l_curr->f : NULL;
}

void  bmiter__loop_of_vert_begin(struct BMIter__loop_of_vert *iter)
{
    ((BMIter *)iter)->count = bmesh_disk_facevert_count(iter->vdata);
    if (((BMIter *)iter)->count) {
        iter->l_first = bmesh_disk_faceloop_find_first(iter->vdata->e, iter->vdata);
        iter->e_first = iter->l_first->e;
        iter->e_next = iter->e_first;
        iter->l_next = iter->l_first;
    }
    else {
        iter->l_first = iter->l_next = NULL;
        iter->e_first = iter->e_next = NULL;
    }
}
void  *bmiter__loop_of_vert_step(struct BMIter__loop_of_vert *iter)
{
    BMLoop *l_curr = iter->l_next;

    if (((BMIter *)iter)->count) {
        ((BMIter *)iter)->count--;
        iter->l_next = bmesh_radial_faceloop_find_next(iter->l_next, iter->vdata);
        if (iter->l_next == iter->l_first) {
            iter->e_next = bmesh_disk_faceedge_find_next(iter->e_next, iter->vdata);
            iter->l_first = bmesh_radial_faceloop_find_first(iter->e_next->l, iter->vdata);
            iter->l_next = iter->l_first;
        }
    }

    if (!((BMIter *)iter)->count) {
        iter->l_next = NULL;
    }

    /* NULL on finish */
    return l_curr;
}

void  bmiter__loop_of_edge_begin(struct BMIter__loop_of_edge *iter)
{
    iter->l_first = iter->l_next = iter->edata->l;
}
void  *bmiter__loop_of_edge_step(struct BMIter__loop_of_edge *iter)
{
    BMLoop *l_curr = iter->l_next;

    if (iter->l_next) {
        iter->l_next = iter->l_next->radial_next;
        if (iter->l_next == iter->l_first) {
            iter->l_next = NULL;
        }
    }

    /* NULL on finish */
    return l_curr;
}

void  bmiter__loop_of_loop_begin(struct BMIter__loop_of_loop *iter)
{
    iter->l_first = iter->ldata;
    iter->l_next = iter->l_first->radial_next;

    if (iter->l_next == iter->l_first)
        iter->l_next = NULL;
}
void  *bmiter__loop_of_loop_step(struct BMIter__loop_of_loop *iter)
{
    BMLoop *l_curr = iter->l_next;

    if (iter->l_next) {
        iter->l_next = iter->l_next->radial_next;
        if (iter->l_next == iter->l_first) {
            iter->l_next = NULL;
        }
    }

    /* NULL on finish */
    return l_curr;
}

void  bmiter__face_of_edge_begin(struct BMIter__face_of_edge *iter)
{
    iter->l_first = iter->l_next = iter->edata->l;
}
void  *bmiter__face_of_edge_step(struct BMIter__face_of_edge *iter)
{
    BMLoop *current = iter->l_next;

    if (iter->l_next) {
        iter->l_next = iter->l_next->radial_next;
        if (iter->l_next == iter->l_first) {
            iter->l_next = NULL;
        }
    }

    return current ? current->f : NULL;
}

void  bmiter__vert_of_edge_begin(struct BMIter__vert_of_edge *iter)
{
    ((BMIter *)iter)->count = 0;
}
void  *bmiter__vert_of_edge_step(struct BMIter__vert_of_edge *iter)
{
    switch (((BMIter *)iter)->count++) {
    case 0:
        return iter->edata->v1;
    case 1:
        return iter->edata->v2;
    default:
        return NULL;
    }
}

void  bmiter__vert_of_face_begin(struct BMIter__vert_of_face *iter)
{
    iter->l_first = iter->l_next = BM_FACE_FIRST_LOOP(iter->pdata);
}
void  *bmiter__vert_of_face_step(struct BMIter__vert_of_face *iter)
{
    BMLoop *l_curr = iter->l_next;

    if (iter->l_next) {
        iter->l_next = iter->l_next->next;
        if (iter->l_next == iter->l_first) {
            iter->l_next = NULL;
        }
    }

    return l_curr ? l_curr->v : NULL;
}

void  bmiter__edge_of_face_begin(struct BMIter__edge_of_face *iter)
{
    iter->l_first = iter->l_next = BM_FACE_FIRST_LOOP(iter->pdata);
}
void  *bmiter__edge_of_face_step(struct BMIter__edge_of_face *iter)
{
    BMLoop *l_curr = iter->l_next;

    if (iter->l_next) {
        iter->l_next = iter->l_next->next;
        if (iter->l_next == iter->l_first) {
            iter->l_next = NULL;
        }
    }

    return l_curr ? l_curr->e : NULL;
}

void  bmiter__loop_of_face_begin(struct BMIter__loop_of_face *iter)
{
    iter->l_first = iter->l_next = BM_FACE_FIRST_LOOP(iter->pdata);
}
void  *bmiter__loop_of_face_step(struct BMIter__loop_of_face *iter)
{
    BMLoop *l_curr = iter->l_next;

    if (iter->l_next) {
        iter->l_next = iter->l_next->next;
        if (iter->l_next == iter->l_first) {
            iter->l_next = NULL;
        }
    }

    return l_curr;
}

int bmesh_disk_facevert_count(const BMVert *v)
{
    /* is there an edge on this vert at all */
    int count = 0;
    if (v->e) {
        BMEdge *e_first, *e_iter;

        /* first, loop around edge */
        e_first = e_iter = v->e;
        do {
            if (e_iter->l) {
                count += bmesh_radial_facevert_count(e_iter->l, v);
            }
        } while ((e_iter = bmesh_disk_edge_next(e_iter, v)) != e_first);
    }
    return count;
}

BMLoop *bmesh_disk_faceloop_find_first(const BMEdge *e, const BMVert *v)
{
    const BMEdge *e_iter = e;
    do {
        if (e_iter->l != NULL) {
            return (e_iter->l->v == v) ? e_iter->l : e_iter->l->next;
        }
    } while ((e_iter = bmesh_disk_edge_next(e_iter, v)) != e);
    return NULL;
}
BMEdge *bmesh_disk_faceedge_find_next(const BMEdge *e, const BMVert *v)
{
    BMEdge *e_find;
    e_find = bmesh_disk_edge_next(e, v);
    do {
        if (e_find->l && bmesh_radial_facevert_check(e_find->l, v)) {
            return e_find;
        }
    } while ((e_find = bmesh_disk_edge_next(e_find, v)) != e);
    return (BMEdge *)e;
}

BMLoop *bmesh_radial_faceloop_find_first(const BMLoop *l, const BMVert *v)
{
    const BMLoop *l_iter;
    l_iter = l;
    do {
        if (l_iter->v == v) {
            return (BMLoop *)l_iter;
        }
    } while ((l_iter = l_iter->radial_next) != l);
    return NULL;
}
BMLoop *bmesh_radial_faceloop_find_next(const BMLoop *l, const BMVert *v)
{
    BMLoop *l_iter;
    l_iter = l->radial_next;
    do {
        if (l_iter->v == v) {
            return l_iter;
        }
    } while ((l_iter = l_iter->radial_next) != l);
    return (BMLoop *)l;
}

int bmesh_radial_facevert_count(const BMLoop *l, const BMVert *v)
{
    const BMLoop *l_iter;
    int count = 0;
    l_iter = l;
    do {
        if (l_iter->v == v) {
            count++;
        }
    } while ((l_iter = l_iter->radial_next) != l);

    return count;
}
bool bmesh_radial_facevert_check(const BMLoop *l, const BMVert *v)
{
    const BMLoop *l_iter;
    l_iter = l;
    do {
        if (l_iter->v == v) {
            return true;
        }
    } while ((l_iter = l_iter->radial_next) != l);

    return false;
}




void BM_mesh_elem_index_ensure(BMesh *bm, const char htype)
{
    const char htype_needed = bm->elem_index_dirty & htype;

#ifdef DEBUG
    BM_ELEM_INDEX_VALIDATE(bm, "Should Never Fail!", __func__);
#endif

    if (htype_needed == 0) {
        goto finally;
    }

    /* skip if we only need to operate on one element */
#pragma omp parallel sections if ((!ELEM(htype_needed, BM_VERT, BM_EDGE, BM_FACE, BM_LOOP, BM_FACE | BM_LOOP)) && \
                                (bm->totvert + bm->totedge + bm->totface >= BM_OMP_LIMIT))
    {
#pragma omp section

        {
            if (htype & BM_VERT) {
                if (bm->elem_index_dirty & BM_VERT) {
                    BMIter iter;
                    BMElem *ele;

                    int index;
                    BM_ITER_MESH_INDEX(ele, &iter, bm, BM_VERTS_OF_MESH, index) {
                        BM_elem_index_set(ele, index); /* set_ok */
                    }
                    BLI_assert(index == bm->totvert);
                }
                else {
                    // printf("%s: skipping vert index calc!\n", __func__);
                }
            }
        }

#pragma omp section
        {
            if (htype & BM_EDGE) {
                if (bm->elem_index_dirty & BM_EDGE) {
                    BMIter iter;
                    BMElem *ele;

                    int index;
                    BM_ITER_MESH_INDEX(ele, &iter, bm, BM_EDGES_OF_MESH, index) {
                        BM_elem_index_set(ele, index); /* set_ok */
                    }
                    BLI_assert(index == bm->totedge);
                }
                else {
                    // printf("%s: skipping edge index calc!\n", __func__);
                }
            }
        }

#pragma omp section
        {
            if (htype & (BM_FACE | BM_LOOP)) {
                if (bm->elem_index_dirty & (BM_FACE | BM_LOOP)) {
                    BMIter iter;
                    BMElem *ele;

                    const bool update_face = (htype & BM_FACE) && (bm->elem_index_dirty & BM_FACE);
                    const bool update_loop = (htype & BM_LOOP) && (bm->elem_index_dirty & BM_LOOP);

                    int index;
                    int index_loop = 0;

                    BM_ITER_MESH_INDEX(ele, &iter, bm, BM_FACES_OF_MESH, index) {
                        if (update_face) {
                            BM_elem_index_set(ele, index); /* set_ok */
                        }

                        if (update_loop) {
                            BMLoop *l_iter, *l_first;

                            l_iter = l_first = BM_FACE_FIRST_LOOP((BMFace *)ele);
                            do {
                                BM_elem_index_set(l_iter, index_loop++); /* set_ok */
                            } while ((l_iter = l_iter->next) != l_first);
                        }
                    }

                    BLI_assert(index == bm->totface);
                    if (update_loop) {
                        BLI_assert(index_loop == bm->totloop);
                    }
                }
                else {
                    // printf("%s: skipping face/loop index calc!\n", __func__);
                }
            }
        }
    }


    finally:
    bm->elem_index_dirty &= ~htype;
}