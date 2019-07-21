#ifndef _CKCOUNTER_H
#define _CKCOUNTER_H

#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <cstring>
#include <string.h>
#include <stdlib.h>
#include "BOBHash.h"

using namespace std;
class CKCounter
{
private:
	struct entry_t {
		char fingerprint;
		uint16_t count;
	};

	struct bucket_t {
		entry_t entry1;// fp:8 bit count:4 bits
		entry_t entry2;// fp:8 bit count:4 bits
		entry_t entry3;//fp:8bits count:8bits
		entry_t entry4;//fp:8bits count:16bits
	};
	int bucket_num, max_loop, h1, h2;
	int k = 8;//the const num
	bucket_t *bucket[2];
	BOBHash * bobhash[2];

public:
	CKCounter(int _bucket) {
		bucket_num = _bucket;
		for (int i = 0; i < 1; i++)
		{
			bobhash[i] = new BOBHash(i + 1000);
		}
		for (int i = 0; i < 2; i++) {
			bucket[i] = new bucket_t[bucket_num];
			memset(bucket[i], 0, sizeof(bucket_t) * bucket_num);
		}
	}
	//swap the  low entry 
	void swap1(entry_t *a, entry_t *b) {// a is the in and b 
		entry_t temp;
		memcpy(&temp, a, sizeof(entry_t));
		memcpy(a, b, sizeof(entry_t));
		memcpy(b, &temp, sizeof(entry_t));
	}

	//the num bigger than the thresh the second insert
	int re_insert(int i, entry_t* reinsert_entry, int hash, int thresh) {
		//uint32_t hash2 = (bobhash[1]->run((const char*)&reinsert_entry.fingerprint, 1)) % bucket_num;
		//int reinsert_hash = (hash ^ hash2) % bucket_num;
		int reinsert_hash = (hash ^ (bobhash[0]->run((const char*)&reinsert_entry->fingerprint, 1))) % bucket_num;
		//printf("the reinserthash1 is =%d,hashfp=%d\n", reinsert_hash, hash2);
		if (bucket[i][reinsert_hash].entry3.fingerprint == NULL) {
			bucket[i][reinsert_hash].entry3.fingerprint = reinsert_entry->fingerprint;
			bucket[i][reinsert_hash].entry3.count = reinsert_entry->count;
			return 1;
		}
		if (bucket[i][reinsert_hash].entry3.count < thresh) {
			swap1(&bucket[i][reinsert_hash].entry3, reinsert_entry);
			rehash(reinsert_entry, max_loop--, i, reinsert_hash);
			return 1;
		}
		if (bucket[i][reinsert_hash].entry4.fingerprint == NULL) {
			bucket[i][reinsert_hash].entry4.fingerprint = reinsert_entry->fingerprint;
			bucket[i][reinsert_hash].entry4.count = reinsert_entry->count;
			return 1;
		}
		if (bucket[i][reinsert_hash].entry4.count < thresh) {
			swap1(reinsert_entry, &bucket[i][reinsert_hash].entry4);
			rehash(reinsert_entry, max_loop--, i, reinsert_hash);
			return 1;
		}
		if (bucket[i][reinsert_hash].entry3.fingerprint != NULL && bucket[i][reinsert_hash].entry4.fingerprint != NULL)
			return 0;
	}


	// insert function  the bit of the entrys is 16, 8, 4, 4
	//insert the new entry
	int insert_new(int i, int hash, char fp) {
	///	entry_low_t inter_entry;
		int jj;
		if (bucket[i][hash].entry1.fingerprint == NULL || bucket[i][hash].entry1.fingerprint == fp) {
			if (bucket[i][hash].entry1.fingerprint == NULL) {
				bucket[i][hash].entry1.fingerprint = fp;
				bucket[i][hash].entry1.count = 0;
			}

			bucket[i][hash].entry1.count++;//if the entry is empty or equl the fp the count++
			if (bucket[i][hash].entry1.count > 16) { //if the count exceed the value of 16 then match it as the other three entry to make the big entry to the first

				if (bucket[i][hash].entry3.count < 16) {//或者为空？？？
					swap1(&bucket[i][hash].entry3, &bucket[i][hash].entry1);
					return 1;
				}
				if (bucket[i][hash].entry4.count < 16) {
					swap1(&bucket[i][hash].entry1, &bucket[i][hash].entry4);
					return 1;
				}
				if (bucket[i][hash].entry3.count > 16 && bucket[i][hash].entry4.count > 16) {
					for (jj = 0; jj < 2; jj++) {
						if (jj != i) {
							if (re_insert(jj,&bucket[i][hash].entry1, hash, 16)) {
								bucket[i][hash].entry1.fingerprint = NULL;//若交换之后直接zhiwei 0了？？？错误应该是直接返回
								bucket[i][hash].entry1.count = 0;
							}
						/*	else {
								bucket[i][hash].entry1.count = 16;
							}*/
						}
						//if other three entrys count are biger than it's thresh ,then we set the min count to the thresh 16
						return 1;
					}
				}
			}
			return 1;
		}
		if (bucket[i][hash].entry2.fingerprint == NULL || bucket[i][hash].entry2.fingerprint == fp) {
			if (bucket[i][hash].entry2.fingerprint == NULL) {
				bucket[i][hash].entry2.fingerprint = fp;
				bucket[i][hash].entry2.count = 0;
			}
			bucket[i][hash].entry2.count++;
			if (bucket[i][hash].entry2.count > 16) {
				if (bucket[i][hash].entry3.count < 16) {
					swap1(&bucket[i][hash].entry3, &bucket[i][hash].entry2);
					return 1;
				}
				if (bucket[i][hash].entry4.count < 16) {
					swap1(&bucket[i][hash].entry2, &bucket[i][hash].entry4);
					return 1;
				}
				if (bucket[i][hash].entry3.count > 16 && bucket[i][hash].entry4.count > 16) {
					for (jj = 0; jj < 2; jj++) {
						if (jj != i) {
							if (re_insert(jj, &bucket[i][hash].entry2, hash, 16)) {
								bucket[i][hash].entry2.fingerprint = NULL;
								bucket[i][hash].entry2.count = 0;
							}
						/*	else {
								bucket[i][hash].entry2.count = 16;
							}*/
						}
						//if other three entrys count are biger than it's thresh ,then we set the min count to the thresh 16
						return 1;
					}
				}
			}
			return 1;
		}
		if (bucket[i][hash].entry3.fingerprint == NULL || bucket[i][hash].entry3.fingerprint == fp) {
			if (bucket[i][hash].entry3.fingerprint == NULL) {
				bucket[i][hash].entry3.fingerprint = fp;
				bucket[i][hash].entry3.count = 0;
			}
			bucket[i][hash].entry3.count++;
			if (bucket[i][hash].entry3.count > 256) {
				if (bucket[i][hash].entry4.count < 256) {
					swap1(&bucket[i][hash].entry3, &bucket[i][hash].entry4);
					return 1;
				}
				if (bucket[i][hash].entry4.count > 256) {
					for (jj = 0; jj < 2; jj++) {
						if (jj != i) {
							if (re_insert(jj, &bucket[i][hash].entry3, hash, 256)) {
								bucket[i][hash].entry3.fingerprint = NULL;
								bucket[i][hash].entry3.count = 0;
							}
						/*	else {
								bucket[i][hash].entry3.count = 256;
							}*/
						}
						//if other three entrys count are biger than it's thresh ,then we set the min count to the thresh 16
						return 1;
					}
				}
			}
			return 1;
		}
		if (bucket[i][hash].entry4.fingerprint == NULL || bucket[i][hash].entry4.fingerprint == fp) {
			if (bucket[i][hash].entry4.fingerprint == NULL) {
				bucket[i][hash].entry4.fingerprint = fp;
				bucket[i][hash].entry4.count = 0;
			}
			bucket[i][hash].entry4.count++;
			return 1;
		}
		if (bucket[i][hash].entry1.fingerprint != fp && bucket[i][hash].entry2.fingerprint != fp && bucket[i][hash].entry3.fingerprint != fp
			&& bucket[i][hash].entry4.fingerprint != fp)
			return 0;
	}

	// reinsert insert the kickout entry
	int rehash_insert(int i, int hash, entry_t* reinsert) {
		if (bucket[i][hash].entry1.fingerprint == NULL) {
			bucket[i][hash].entry1.fingerprint = reinsert->fingerprint;
			bucket[i][hash].entry1.count = reinsert->count;
			return 1;
		}
		if (bucket[i][hash].entry2.fingerprint == NULL) {

			bucket[i][hash].entry2.fingerprint = reinsert->fingerprint;
			bucket[i][hash].entry2.count = reinsert->count;
			return 1;
		}
		if (bucket[i][hash].entry3.fingerprint == NULL) {
			bucket[i][hash].entry3.fingerprint = reinsert->fingerprint;
			bucket[i][hash].entry3.count = reinsert->count;
			return 1;
		}
		if (bucket[i][hash].entry4.fingerprint == NULL) {
			bucket[i][hash].entry4.fingerprint = reinsert->fingerprint;
			bucket[i][hash].entry4.count = reinsert->count;
			return 1;
		}

		if (bucket[i][hash].entry1.fingerprint != NULL && bucket[i][hash].entry2.fingerprint != NULL && bucket[i][hash].entry3.fingerprint != NULL
			&& bucket[i][hash].entry4.fingerprint != NULL)
			return 0;
	}

	// if the item is kickout 
	bool rehash(entry_t* hash_entry, int loop_times, int j, int hash) {
		int k;
		//uint32_t hash2 = (bobhash[1]->run((const char*)&hash_entry.fingerprint, 1)) % bucket_num;
		//int re_hash = (hash ^ hash2) % bucket_num;
		int re_hash = (hash ^ (bobhash[0]->run((const char*)&hash_entry->fingerprint, 1))) % bucket_num;
		//printf("the rehash1 is =%d,hashfp=%d\n", re_hash, hash2);
		if (loop_times == 0) {
			for (int k = 0; k < 2; k++) {
				if (k != j) {
					bucket[k][re_hash].entry1.fingerprint = hash_entry->fingerprint;//if reflash the fingerprint or not 
					bucket[k][re_hash].entry1.count += hash_entry->count;//if the loop times biger than the thresh 
																		//then we add the count to the bucket1 the min entry in a erro for the entry be added
					if (bucket[k][re_hash].entry1.count <= 16)
						return true;
					else {
						if (bucket[k][re_hash].entry3.count < 16)
							swap1(&bucket[k][re_hash].entry1, &bucket[k][re_hash].entry3);
						else if (bucket[k][re_hash].entry4.count < 16)
							swap1(&bucket[k][re_hash].entry1, &bucket[k][re_hash].entry4);
						else
							bucket[k][re_hash].entry1.count = hash_entry->count;						
						return true;
					}			
				}
			}
		}
		for (int h = 0; h < 2; h++) {
			if (h != j) {
				int res = rehash_insert(h, re_hash, hash_entry);
				if (res == 1)
					return true;
				else {
					swap1(&bucket[h][re_hash].entry1, hash_entry);
					loop_times--;
				// 	printf("the loop_times = %6d\n", loop_times);
					if (rehash(hash_entry, loop_times, h, re_hash)) {
						return true;
					}
				}
			}
		}
	}

	void Insert(char *key) {
		int  res;
		entry_t inter_entry;//we sure the kickout is not the bigesst one
		max_loop = 2;//the bigest loop times()
		h1 = (bobhash[0]->run(key, strlen(key))) % bucket_num;		
		char fp;
		fp = (char)((int)*key ^ ((int)*key >> 8) ^ ((int)*key >> 16) ^ ((int)*key >> 24));
		//uint32_t hash_fp = (bobhash[1]->run((const char*)&fp, 1)) % bucket_num; // the hash(f)
		//h2 = (h1 ^ hash_fp) % bucket_num;
		h2 = (h1 ^ (bobhash[0]->run((const char*)&fp, 1))) % bucket_num;//chenged in 20190626
		int hash[2] = { h1, h2 };
		for (int i = 0; i < 2; i++) {
	//		if (hash[i] < bucket_num) {
				res = insert_new(i, hash[i], fp);//insert to the bucket if the bucket not full res=1,else the res=0 then rehash
				if (res) {
					return;
				}
		//	}
			
		}
		if (res == 0) {//if bucket1 and bucket2 all full then kickout
			int j = rand() % 2;//select a bucket to rehash
			inter_entry.fingerprint = bucket[j][hash[j]].entry1.fingerprint;
			inter_entry.count = bucket[j][hash[j]].entry1.count;
			bucket[j][hash[j]].entry1.fingerprint = fp;
			bucket[j][hash[j]].entry1.count = 1;
			if (rehash(&inter_entry, max_loop--, j, hash[j])) {
				return;
			}
		}
	//	return;
	}

	// given an key to find it's count in the sketch
	double Query(char *key) {
		int j;
		h1 = (bobhash[0]->run(key, strlen(key))) % bucket_num;
		char fp;
		fp = (char)((int)*key ^ ((int)*key >> 8) ^ ((int)*key >> 16) ^ ((int)*key >> 24));
		//	fp = (char *)&h1;
		//uint32_t hash_fp = (bobhash[1]->run((const char*)&fp, 1)) % bucket_num;
		//h2 = (h1 ^ hash_fp) % bucket_num;
		h2 = (h1 ^ (bobhash[0]->run((const char*)&fp, 1))) % bucket_num;
		int query_hash[2] = { h1,h2 };
		for (int i = 0; i < 2; i++) {
			if (bucket[i][query_hash[i]].entry1.fingerprint == fp)
				return bucket[i][query_hash[i]].entry1.count;// / (1 + (bucket[i][query_hash[i]].entry1.count / k))
			if (bucket[i][query_hash[i]].entry2.fingerprint == fp)
				return bucket[i][query_hash[i]].entry2.count;
			if (bucket[i][query_hash[i]].entry3.fingerprint == fp)
				return bucket[i][query_hash[i]].entry3.count;
			if (bucket[i][query_hash[i]].entry4.fingerprint == fp)
				return bucket[i][query_hash[i]].entry4.count;

		}
		if (bucket[1][query_hash[1]].entry1.fingerprint != fp && bucket[1][query_hash[1]].entry2.fingerprint != fp
			&& bucket[1][query_hash[1]].entry3.fingerprint != fp && bucket[1][query_hash[1]].entry4.fingerprint != fp
			&& bucket[2][query_hash[2]].entry1.fingerprint != fp && bucket[2][query_hash[2]].entry2.fingerprint != fp
			&& bucket[2][query_hash[2]].entry3.fingerprint != fp && bucket[2][query_hash[2]].entry4.fingerprint != fp) {
			j = rand() % 2;
			return bucket[j][query_hash[j]].entry1.count / (1+(bucket[j][query_hash[j]].entry1.count/k));//if not find return the bucket1 min count ,we think the count is extra the maxloop it added in the min count
		//	return bucket[j][query_hash[j]].entry1.count;
		}
	}
	//memeory access
	double Mem(char *key) {
		int j = 0;
		h1 = (bobhash[0]->run(key, strlen(key))) % bucket_num;
		char fp;
		fp = (char)((int)*key ^ ((int)*key >> 8) ^ ((int)*key >> 16) ^ ((int)*key >> 24));
		//uint32_t hash_fp = (bobhash[1]->run((const char*)&fp, 1)) % bucket_num;
		//h2 = (h1 ^ hash_fp) % bucket_num;
		h2 = (h1 ^ (bobhash[0]->run((const char*)&fp, 1))) % bucket_num;
		int memory_hash[2] = { h1,h2 };
		for (int i = 0; i < 2; i++) {
			if (bucket[i][memory_hash[i]].entry1.fingerprint == fp) {
				j++;
				return j;
			}
			if (bucket[i][memory_hash[i]].entry2.fingerprint == fp) {
				j++;
				return j;
			}

			if (bucket[i][memory_hash[i]].entry3.fingerprint == fp) {
				j++;
				return j;
			}

			if (bucket[i][memory_hash[i]].entry4.fingerprint == fp) {
				j++;
				return j;
			}

		}
		if (bucket[1][memory_hash[1]].entry1.fingerprint != fp && bucket[1][memory_hash[1]].entry2.fingerprint != fp
			&& bucket[1][memory_hash[1]].entry3.fingerprint != fp && bucket[1][memory_hash[1]].entry4.fingerprint != fp
			&& bucket[2][memory_hash[2]].entry1.fingerprint != fp && bucket[2][memory_hash[2]].entry2.fingerprint != fp
			&& bucket[2][memory_hash[2]].entry3.fingerprint != fp && bucket[2][memory_hash[2]].entry4.fingerprint != fp) {
			j += 2;
			return j;//if not find return the bucket1 min count ,we think the count is extra the maxloop it added in the min count
		}
	}

	// the use ratio of the bucket
	double Ratio() {
		int used_entry = 0;
		int i, j;
		for (i = 0; i < 2; i++) {
			for (j = 0; j < bucket_num; j++) {
				if (bucket[1][j].entry1.count != 0)
					used_entry++;
				if (bucket[1][j].entry2.count != 0)
					used_entry++;
				if (bucket[1][j].entry3.count != 0)
					used_entry++;
				if (bucket[1][j].entry4.count != 0)
					used_entry++;
			}
		}
		return used_entry / (bucket_num * 2.0);
	}

	//delete the bucket
	void Delete(char *key) {
		h1 = (bobhash[0]->run(key, strlen(key))) % bucket_num;
		char fp;
		fp = (char)((int)*key ^ ((int)*key >> 8) ^ ((int)*key >> 16) ^ ((int)*key >> 24));
		//uint32_t hash_fp = (bobhash[1]->run((const char*)&fp, 1)) % bucket_num;
	  //  h2 = (h1 ^ hash_fp) % bucket_num;
		h2 = (h1 ^ (bobhash[0]->run((const char*)&fp, 1))) % bucket_num;
		int delet_hash[2] = { h1,h2 };
		for (int i = 0; i < 2; i++) {
			if (bucket[i][delet_hash[i]].entry1.fingerprint == fp)
			{
				//	bucket[i][delet_hash[i]].entry1.fingerprint = NULL;
				bucket[i][delet_hash[i]].entry1.count--;
				return;
			}
			if (bucket[i][delet_hash[i]].entry2.fingerprint == fp) {
				//	bucket[i][delet_hash[i]].entry2.fingerprint = NULL;
				bucket[i][delet_hash[i]].entry2.count--;
				return;
			}
			if (bucket[i][delet_hash[i]].entry3.fingerprint == fp) {
				//bucket[i][delet_hash[i]].entry3.fingerprint = NULL;
				bucket[i][delet_hash[i]].entry3.count--;
				return;
			}
			if (bucket[i][delet_hash[i]].entry4.fingerprint == fp) {
				//bucket[i][delet_hash[i]].entry4.fingerprint = NULL    ;
				bucket[i][delet_hash[i]].entry4.count--;
				return;
			}
		}
	}

	~CKCounter() {
		for (int i = 0; i < 2; i++) {
			delete[]bucket[i];
		}
		for (int i = 0; i < 2; i++) {
			delete bobhash[i];
		}
	}
};
#endif//_CKCOUNTER_H