#include<stdio.h>
#include<stdlib.h>
#include<string.h>

// 사용자 정의 헤더 파일
#include "list.h"
#include "hash.h"
#include "bitmap.h"

// 정적 함수 선언
static bool func_list_less(const struct list_elem* n1, const struct list_elem* n2, void* aux);
static unsigned hashfunc(const struct hash_elem* h, void* aux);
static bool func_hash_less(const struct hash_elem* n1, const struct hash_elem* n2, void* aux);


int main() {
	char* user_input = (char*)malloc(40 * sizeof(char));//user input

	struct list_elem* e;
	struct list_elem* elem1, * elem2, * new_elem;
	struct list list_array[10];

	struct hash_iterator hash_iter;
	struct hash_elem* h, * new_hash;
	struct hash hash_array[10];


	struct bitmap* btm;
	struct bitmapitem bit_array[10];
		
	unsigned long long ui;
	int n1, n2, n3, i;		
	int in_first, in_second;//array index input
	char* l;


	while (1) {
		fgets(user_input, 50, stdin);
		l = strchr(user_input, '\n');
		if (l  != NULL) *l = '\0';
		if (strcmp(user_input, "quit")==0)
			break;

		//삭제 처리
		else if (strncmp(user_input, "delete list", 11)==0) {
			
			sscanf(user_input, "delete list %d", &in_first);
			while (!list_empty(&list_array[in_first]))
				list_pop_front(&list_array[in_first]);
		}
		else if (strncmp(user_input, "delete hash", 11)==0) {
			sscanf(user_input, "delete hash %d", &in_first);
			hash_destroy(&hash_array[in_first], NULL);
		}
		else if (strncmp(user_input, "delete bm", 9)==0) {
			sscanf(user_input, "delete bm %d", &in_first);
			bitmap_destroy(bit_array[in_first].bitm);
			bit_array[in_first].s = 0;
		}
		// 생성 처리
		else if (strncmp(user_input, "create list list", 16)==0) {
			sscanf(user_input, "create list list %d", &in_first);
			list_init(&list_array[in_first]);
		}

		else if (strncmp(user_input, "create hashtable hash", 21)==0) {
			sscanf(user_input, "create hashtable hash %d", &in_first);
			hash_init(&hash_array[in_first], &hashfunc, &func_hash_less, NULL);
		}
		else if (strncmp(user_input, "create bitmap bm", 16)==0) {
			sscanf(user_input, "create bitmap bm %d %d", &in_first, &n1);
			bit_array[in_first].bitm = bitmap_create(n1);
			bit_array[in_first].s = 0;
		}
		//출력 처리
		else if (strncmp(user_input, "dumpdata list", 13)==0) {
			sscanf(user_input, "dumpdata list %d", &in_first);
			if (!list_empty(&list_array[in_first])) {
        		for (e = list_begin(&list_array[in_first]); e != list_end(&list_array[in_first]); e = list_next(e)) {
            		printf("%d ", (list_entry(e, struct listitem, elem))->data);
       			}
        	printf("\n");
    		}
		}
		else if (strncmp(user_input, "dumpdata hash", 13)==0) {
			sscanf(user_input, "dumpdata hash %d", &in_first);
			struct hash_iterator hash_iter;
   			hash_first(&hash_iter, &hash_array[in_first]);

   			while (hash_next(&hash_iter)) {
        		struct hashitem *item = hash_entry(hash_cur(&hash_iter), struct hashitem, elem);
        		printf("%d ", item->data);
    		}
    		printf("\n");
		}
		// 비트맵 데이터 출력
		else if (strncmp(user_input, "dumpdata bm", 11)==0) {
			sscanf(user_input, "dumpdata bm %d", &in_first);
  		 	btm = bit_array[in_first].bitm;
    		n1 = bitmap_size(btm);
			for (i = 0; i < n1; i++)
				printf("%d", bitmap_test(btm, i));
			printf("\n");
					
		}
		// 비트맵 검사 
		else if (strncmp(user_input, "bitmap_all bm", 13)==0) {
			sscanf(user_input, "bitmap_all bm %d %d %d", &in_first, &n1, &n2);
			btm = bit_array[in_first].bitm;
			if (!bitmap_all(btm, n1, n2)) printf("false\n");
			else    printf("true\n");
		}
		// 비트맵 마킹
		else if (strncmp(user_input, "bitmap_mark bm", 14)==0) {
			sscanf(user_input, "bitmap_mark bm %d %d", &in_first, &n1);
			btm = bit_array[in_first].bitm;
			// 특정 위치의 비트를 설정
			bitmap_mark(btm, n1);
			bit_array[in_first].s++;
		}
		// 비트맵 포함 여부
		else if (strncmp(user_input, "bitmap_contains bm", 18)==0) {
			sscanf(user_input, "bitmap_contains bm %d %d %d %s", &in_first, &n1, &n2, user_input);
			btm = bit_array[in_first].bitm;
			// 비트맵이 특정 패턴을 포함하는지 확인
			if (strncmp(user_input, "false", 5)==0)
				i = bitmap_contains(btm, n1, n2, false);
			else if (strncmp(user_input, "true", 4)==0)
				i = bitmap_contains(btm, n1, n2, true);
			else {
				continue;
			}
			if (i == true){
				printf("true\n");
			}
			else{
				printf("false\n");
			}    
		}
		// 비트맵 리셋
		else if (strncmp(user_input, "bitmap_reset bm", 15)==0) {
			sscanf(user_input, "bitmap_reset bm %d %d", &in_first, &n1);
			btm = bit_array[in_first].bitm;
			bitmap_reset(btm, n1);
		}
		// 비트맵 카운트
		else if (strncmp(user_input, "bitmap_count bm", 15)==0) {
			sscanf(user_input, "bitmap_count bm %d %d %d %s", &in_first, &n1, &n2, user_input);
			btm = bit_array[in_first].bitm;
			if (!strncmp(user_input, "true", 4))
				i = bitmap_count(btm, n1, n2, true);
			else if (!strncmp(user_input, "false", 5))
				i = bitmap_count(btm, n1, n2, false);
			else {
				continue;
			}
			printf("%d\n", i);
		}
		//n1부터 n2까지 비어있는지 확인
		else if (strncmp(user_input, "bitmap_none bm", 14)==0) {
			sscanf(user_input, "bitmap_none bm %d %d %d", &in_first, &n1, &n2);
			btm = bit_array[in_first].bitm;
			if (!bitmap_none(btm, n1, n2)) printf("false\n");
			else    printf("true\n");
		}
		 // 비트맵 덤프
		else if (strncmp(user_input, "bitmap_dump bm", 14)==0) {
			sscanf(user_input, "bitmap_dump bm %d", &in_first);
			struct bitmapitem* tb1 = bit_array[in_first].bitm;
			bitmap_dump(tb1);
		}
		else if (strncmp(user_input, "bitmap_any bm", 13)==0) {
			sscanf(user_input, "bitmap_any bm %d %d %d", &in_first, &n1, &n2);
			btm = bit_array[in_first].bitm;
			if (!bitmap_any(btm, n1, n2)) printf("false\n");
			else    printf("true\n");
		}
		 // 비트맵 반전
		else if (strncmp(user_input, "bitmap_flip bm", 14)==0) {
			sscanf(user_input, "bitmap_flip bm %d %d", &in_first, &n1);
			btm = bit_array[in_first].bitm;
			bitmap_flip(btm, n1);
		}
		// 비트맵 설정
		else if (strncmp(user_input, "bitmap_set bm", 13)==0) {
			sscanf(user_input, "bitmap_set bm %d %d %s", &in_first, &n1, user_input);
			btm = bit_array[in_first].bitm;
			if (strncmp(user_input, "false", 5)==0){
				bitmap_set(btm, n1, false);
			}
			else if (strncmp(user_input, "true", 4)==0){
				bitmap_set(btm, n1, true);
			}
			else continue;
		}
		// 비트맵 스캔 및 반전
		else if (strncmp(user_input, "bitmap_scan_and_flip bm", 23)==0) {
			sscanf(user_input, "bitmap_scan_and_flip bm %d %d %d %s", &in_first, &n1, &n2, user_input);
			btm = bit_array[in_first].bitm;
			if (strncmp(user_input, "false", 5)==0)
				ui = bitmap_scan_and_flip(btm, n1, n2, false);
			else if (strncmp(user_input, "true", 4)==0)
				ui = bitmap_scan_and_flip(btm, n1, n2, true);
			printf("%zu\n", ui);//unsigned int
		}
		// 비트맵 확장
		else if (strncmp(user_input, "bitmap_expand bm", 16)==0) {
			sscanf(user_input, "bitmap_expand bm %d %d", &in_first, &n1);
			btm = bit_array[in_first].bitm;
			bit_array[in_first].bitm = bitmap_expand(btm, n1);
		}
		 // 비트맵 스캔
		else if (strncmp(user_input, "bitmap_scan bm", 14)==0) {
			sscanf(user_input, "bitmap_scan bm %d %d %d %s", &in_first, &n1, &n2, user_input);
			btm = bit_array[in_first].bitm;
			if (!strncmp(user_input, "false", 5))
				ui = bitmap_scan(btm, n1, n2, false);
			else if (!strncmp(user_input, "true", 4))
				ui = bitmap_scan(btm, n1, n2, true);
			printf("%zu\n", ui);
		}
		else if (strncmp(user_input, "bitmap_set_all bm", 17)==0) {
			sscanf(user_input, "bitmap_set_all bm %d %s", &in_first, user_input);
			btm = bit_array[in_first].bitm;
			if (!strncmp(user_input, "true", 4))
				bitmap_set_all(btm, true);
			else if (!strncmp(user_input, "false", 5))
				bitmap_set_all(btm, false);
			else continue;
		}
		//n1이 있는지 확인
		else if (strncmp(user_input, "bitmap_test bm", 14)==0) {
			sscanf(user_input, "bitmap_test bm %d %d", &in_first, &n1);
			btm = bit_array[in_first].bitm;
			if (!bitmap_test(btm, n1))
				printf("false\n");
			else    printf("true\n");
		}
		else if (strncmp(user_input, "bitmap_set_multiple bm", 22)==0) {
			sscanf(user_input, "bitmap_set_multiple bm %d %d %d %s", &in_first, &n1, &n2, user_input);
			btm = bit_array[in_first].bitm;
			if (strncmp(user_input, "false", 5)==0){
				bitmap_set_multiple(btm, n1, n2, false);
			}
			else if (strncmp(user_input, "true", 4)==0){
				bitmap_set_multiple(btm, n1, n2, true);
			}
			else    continue;
		}
		 // 비트맵 크기
		else if (strncmp(user_input, "bitmap_size bm", 14)==0) {
			sscanf(user_input, "bitmap_size bm %d", &in_first);
			btm = bit_array[in_first].bitm;
			printf("%d\n", bitmap_size(btm));
		}
		// 리스트 스왑
		else if (strncmp(user_input, "list_swap list", 14)==0) {
			sscanf(user_input, "list_swap list %d %d %d", &in_first, &n1, &n2);
			elem1 = list_begin(&list_array[in_first]);
			elem2 = list_begin(&list_array[in_first]);
		
    		for (i = 0; i < n1; i++) {
        		elem1 = list_next(elem1); //n1의 위치
   			 }
 			for (i = 0; i < n2; i++) {
      			  elem2 = list_next(elem2); //n2의 위치
   			 }
    			list_swap(elem1, elem2); //swap
		}
		// 리스트 셔플
		else if (strncmp(user_input, "list_shuffle list", 17)==0) {
			sscanf(user_input, "list_shuffle list %d", &in_first);
			list_shuffle(&list_array[in_first]);
			
		}
		// 리스트 크기
		else if (strncmp(user_input, "list_size list", 14)==0) {
			sscanf(user_input, "list_size list %d", &in_first);
			printf("%u\n", list_size(&list_array[in_first]));
		}
		// 리스트 뒤 요소
		else if (strncmp(user_input, "list_back list", 14)==0) {
			sscanf(user_input, "list_back list %d", &in_first);
			e = list_rbegin(&list_array[in_first]);
			printf("%d\n", (list_entry(e, struct listitem, elem))->data);
		}
		// 리스트 뒤에 요소 추가
		else if (strncmp(user_input, "list_push_back list", 19)==0) {
			sscanf(user_input, "list_push_back list %d %d", &in_first, &n1);
			new_elem = make_listelem(n1);
			list_push_back(&list_array[in_first], new_elem);
		}
		// 리스트 뒤에 요소 pop
		else if (strncmp(user_input, "list_pop_back list", 18)==0) {
			sscanf(user_input, "list_pop_back list %d", &in_first);
			list_pop_back(&list_array[in_first]);
		}
		// empty인지 확인
		else if (strncmp(user_input, "list_empty list", 15)==0) {
			sscanf(user_input, "list_empty list %d", &in_first);
			if (!list_empty(&list_array[in_first]))
				printf("false\n");
			else    printf("true\n");
		}
		else if (strncmp(user_input, "list_splice list", 16)==0) {
			sscanf(user_input, "list_splice list %d %d list %d %d %d", &in_first, &n1, &in_second, &n2, &n3);
			//리스트에서 요소 위치를 설정
			e = list_begin(&list_array[in_first]);
			elem1 = list_begin(&list_array[in_second]);
			elem2 = list_begin(&list_array[in_second]);
			for (i = 0; i < n3; i++) {
				if (i < n2)   elem1 = list_next(elem1);
				elem2 = list_next(elem2);
			}
			for (i = 0; i < n1; i++)  {
				e = list_next(e);
			}
			 // splice
			list_splice(e, elem1, elem2);
		}
		else if (strncmp(user_input, "list_unique list", 16)==0) {
			in_second = -20;
			// 중복을 제거
			sscanf(user_input, "list_unique list %d list %d", &in_first, &in_second);
			if (in_second == -20)
				list_unique(&list_array[in_first], NULL, &func_list_less, NULL);
			else {
				list_unique(&list_array[in_first], &list_array[in_second], &func_list_less, NULL);
			}
		}
		 // 리스트의 맨 앞에 요소를 추가
		else if (strncmp(user_input, "list_push_front list", 20)==0) {
			sscanf(user_input, "list_push_front list %d %d", &in_first, &n1);
			new_elem = make_listelem(n1);
			list_push_front(&list_array[in_first], new_elem);
		}
		// 리스트의 맨 앞 요소
		else if (strncmp(user_input, "list_front list", 15)==0) {
			sscanf(user_input, "list_front list %d", &in_first);
			e = list_begin(&list_array[in_first]);
			printf("%d\n", (list_entry(e, struct listitem, elem))->data);
		}
		// 리스트의 맨 앞 요소 pop
		else if (strncmp(user_input, "list_pop_front list", 19)==0) {
			sscanf(user_input, "list_pop_front list %d", &in_first);
			list_pop_front(&list_array[in_first]);
		}
		else if (strncmp(user_input, "list_remove list", 16)==0) {
			sscanf(user_input, "list_remove list %d %d", &in_first, &n1);
			e = list_begin(&list_array[in_first]);
			for (i = 0; i < n1; i++) {
				e = list_next(e);
			}
			list_remove(e);
		}
		//리스트에 값 insert
		else if (strncmp(user_input, "list_insert list", 16)==0) {
			sscanf(user_input, "list_insert list %d %d %d", &in_first, &n1, &n2);
			new_elem = make_listelem(n2);
			e = list_begin(&list_array[in_first]);
			for (i = 0; i < n1; i++) e = list_next(e);
			list_insert(e, new_elem);
		}
		//정렬되도록 insert
		else if (strncmp(user_input, "list_insert_ordered list", 24)==0) {
			sscanf(user_input, "list_insert_ordered list %d %d", &in_first, &n1);
			new_elem = make_listelem(n1);
			list_insert_ordered(&list_array[in_first], new_elem, &func_list_less, NULL);
		}
		//리스트 반전
		else if (strncmp(user_input, "list_reverse list", 17)==0) {
			sscanf(user_input, "list_reverse list %d", &in_first);
			list_reverse(&list_array[in_first]);
		}
		//max값
		else if (strncmp(user_input, "list_max list", 13)==0) {
			sscanf(user_input, "list_max list %d", &in_first);
			e = list_max(&list_array[in_first], &func_list_less, NULL);
			printf("%d\n", (list_entry(e, struct listitem, elem))->data);
		}
		//리스트 정렬
		else if (strncmp(user_input, "list_sort list", 14)==0) {
			sscanf(user_input, "list_sort list %d", &in_first);
			list_sort(&list_array[in_first], &func_list_less, NULL);
		}
		//min값
		else if (strncmp(user_input, "list_min list", 13)==0) {
			sscanf(user_input, "list_min list %d", &in_first);
			e = list_min(&list_array[in_first], &func_list_less, NULL);
			printf("%d\n", (list_entry(e, struct listitem, elem))->data);
		}
		 // 해시 테이블 엔트리를 제거
		else if (strncmp(user_input, "hash_clear hash", 15)==0) {
			sscanf(user_input, "hash_clear hash %d", &in_first);
			hash_clear(&hash_array[in_first], NULL);
		}
		// 해시 테이블에서 특정 엔트리를 삭제
		else if (strncmp(user_input, "hash_delete hash", 16)==0) {
			sscanf(user_input, "hash_delete hash %d %d", &in_first, &n1); 

			struct hashitem delete_elem;
			delete_elem.data = n1;
			// 삭제할 엔트리 찾기
			struct hash_elem* found_elem = hash_find(&hash_array[in_first], &delete_elem.elem);
			// 엔트리를 찾았을 경우 
			if (found_elem != NULL) 
				hash_delete(&hash_array[in_first], found_elem);
		}
		// 해시 테이블에 새로운 엔트리 삽입
		else if (strncmp(user_input, "hash_insert hash", 16)==0) {
			sscanf(user_input, "hash_insert hash %d %d", &in_first, &n1);
			new_hash = make_hashelem(n1);
			hash_insert(&hash_array[in_first], new_hash);
		}
		// 해시 테이블의 크기
		else if (strncmp(user_input, "hash_size hash", 14)==0) {
			sscanf(user_input, "hash_size hash %d", &in_first);
			printf("%d\n", hash_size(&hash_array[in_first]));
		}
		else if (strncmp(user_input, "hash_replace hash", 17)==0) {
			sscanf(user_input, "hash_replace hash %d %d", &in_first, &n1);
			new_hash = make_hashelem(n1);
			hash_replace(&hash_array[in_first], new_hash);
		}
		// 해시 테이블이 비어있는지 여부
		else if (strncmp(user_input, "hash_empty hash", 15)==0) {
			sscanf(user_input, "hash_empty hash %d", &in_first);
			if (hash_empty(&hash_array[in_first]))
				printf("true\n");
			else    printf("false\n");
		}
		// 해시 테이블에서 특정 값 찾기
		else if (strncmp(user_input, "hash_find hash", 14)==0) {
			sscanf(user_input, "hash_find hash %d %d", &in_first, &n1);

			struct hashitem find_elem;
			find_elem.data = n1; 
			struct hash_elem *found_elem = hash_find(&hash_array[in_first], &find_elem.elem);
			
			struct hashitem *found_item = hash_entry(found_elem, struct hashitem, elem);
			if(found_elem!=NULL) printf("%d\n", found_item->data);

		}
		 // 입력한 액션에 따라 해시 테이블에 액션을 적용(세제곱 or 제곱)
		else if (strncmp(user_input, "hash_apply hash", 15)==0) {
			n1 = 0;
			sscanf(user_input, "hash_apply hash %d %s", &in_first, user_input);
			if (!strncmp(user_input, "triple", 6))	    n1 = 1;
			else if (!strncmp(user_input, "square", 6)) n1 = 2;
			else    continue;
			 hash_first(&hash_iter, &hash_array[in_first]);

   			if (n1 == 1)
				hash_apply(&hash_array[in_first], triple_action);
			else
				hash_apply(&hash_array[in_first], square_action);
		}		


		else
			printf("error\n");


	}

	return 0;
}
static bool func_list_less(const struct list_elem* n1, const struct list_elem* n2, void* aux) {
	int a = (list_entry(n1, struct listitem, elem))->data;
	int b = (list_entry(n2, struct listitem, elem))->data;

	return a < b;
}
static unsigned hashfunc(const struct hash_elem* h, void* aux) {
	int n = (hash_entry(h, struct hashitem, elem))->data;
	return hash_int(n);
}

static bool func_hash_less(const struct hash_elem* n1, const struct hash_elem* n2, void* aux) {
	int a = (hash_entry(n1, struct hashitem, elem))->data;
	int b = (hash_entry(n2, struct hashitem, elem))->data;

	return a < b;
}


