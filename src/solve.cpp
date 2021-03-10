#include"solve.h"

namespace dzh{
	void SAT::init(clauseNode* head, word*& word_) {
		for (int i = 0; i < variableNum; i++) {
			word_[i].truthValue = -1;
			word_[i].frequency = 0;
			word_[i].frequency_bak = 0;
			word_[i].polarity = 0;
			word_[i].decesionLevel = -1;
			word_[i].antecedent = nullptr;
		}
		clauseNode* p = head->down;
		while (p) {
			wordNode* q = p->right;
			while (q) {
				int var = abs(q->num) - 1;
				word_[var].frequency++;
				word_[var].frequency_bak++;
				word_[var].polarity += q->num > 0 ? 1 : -1;
				q = q->next;
			}
			p = p->down;
		}
		return;
	}
	word* SAT::solve(std::string path, SAT*& solver) {

		cnfData* loader = new cnfData();
		clauseNode* head = loader->cnfParse(path);
		solver->variableNum = head->wordNum;
		word* word_ = (word*)malloc(sizeof(word) * head->wordNum);
		solver->init(head, word_);
		clock_t start = clock();
		status status_ = solver->DPLL(head, word_);
		clock_t end = clock();
		delete loader;
		return word_;
	}

	/*
	1. �ҵ���λ�Ӿ�
	2. ɾ�����к��е�λ�Ӿ����ֵ��־�
	3. ɾ�����к��е�λ�Ӿ为���ֵ�����
	4. ����1-3��û�е�λ�Ӿ�
	5. ����ʽ��������ֵһ������
	*/
	status SAT::DPLL(clauseNode* head, word*& word_) {
		status status_ = normal;
		status_ = unitPropagation(head, word_);		// ��Ԫ�Ӿ䴫��
		// std::cout << "up in dpll status:" << status_ << std::endl;
		if (status_ == satisfied) {
			return success;
		}
		else if (status_ == unsatisfied) {
			return normal;
		}
		//TODO ��������ʽ
		int var = -1;
		bool isFind = false;
		clauseNode* p = head->down;
		while (p && !isFind) {
			wordNode* q = p->right;
			while (q && !isFind) {
				int num = abs(q->num) - 1;
				if (word_[num].frequency > 0) {	/*TODO �ҵ�Ƶ����ߵ����� ע��Ҫ-1*/
					var = num;
					isFind = true;
				}
				q = q->next;
			}
			p = p->down;
		}
		// std::cout << "choose var: " << var << std::endl;

		for (int i = 1; i >= 0; i--) {
			word* cpy = cnfData::clone(word_, variableNum);
			// ����head
			clauseNode* headCpy = cnfData::clone(head);		//TODO
			if (cpy[var].polarity > 0) {
				cpy[var].truthValue = i;
			}
			else {
				cpy[var].truthValue = (i + 1) % 2;
			}
			cpy[var].frequency = -1;
			status_ = applyAssignment(headCpy, cpy[var].truthValue ? var+1 : -var-1);
			// std::cout << "aa in dpll status:" << status_ << std::endl;
			if (status_ == satisfied) {
				//displayResult(cpy, head->wordNum);
				return success;
			}
			else if (status_ == unsatisfied) {
				// TODO free cpy and headCpy
				free(cpy);
				while (headCpy->down) {
					cnfData::distroyClause(headCpy, headCpy->down);
				}
				free(headCpy);
				continue;
			}
			status_ = DPLL(headCpy, cpy);
			if (status_ == success) {
				word_ = cnfData::clone(cpy, variableNum);
				free(cpy);
				while (headCpy && headCpy->down) {
					cnfData::distroyClause(headCpy, headCpy->down);
				}
				free(headCpy);
				return success;
			}
			// TODO free cpy and headCpy
			free(cpy);
			while (headCpy && headCpy->down) {
				cnfData::distroyClause(headCpy, headCpy->down);
			}
			free(headCpy);

		}
		return normal;
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="head"></param>
	/// <param name="word_"></param>
	/// <returns>
	///		satisfied	������
	///		unsatisfied	�����㣬����
	///		normal		��ʱû�ҵ��������
	/// </returns>
	status SAT::unitPropagation(clauseNode*& head, word* word_) {
		bool hasUnit = false;
		clauseNode* p = nullptr;
		if (head->down == nullptr) {		// ���־伯
			return satisfied;
		}
		status status_;
		do {
			hasUnit = false;
			p = head->down;

			while (p && !hasUnit) {
				if (p->wordNum == 1) {		// �ҵ��˵��Ӿ�
					hasUnit = true;
					word_[abs(p->right->num) - 1].truthValue = p->right->num > 0 ? 1 : 0;
					word_[abs(p->right->num) - 1].frequency = -1;
					status_ = applyAssignment(head, p->right->num);
					// std::cout << "aa in up status:" << status_ << std::endl;
					if (status_ == satisfied || status_ == unsatisfied) {
						return status_;
					}
					break;
				}
				else if(p->wordNum == 0){	// �ҵ��˿��Ӿ�
					return unsatisfied;
				}
				p = p->down;
			}

		} while (hasUnit);
		return normal;
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="head"></param>
	/// <param name="num"></param>
	/// <returns>
	///		satisfied	������
	///		unsatisfied	�����㣬����
	///		normal		��ʱû�ҵ��������
	/// </returns>
	status SAT::applyAssignment(clauseNode* head, int num) {
		clauseNode* p = head;
		while (p) {
			wordNode* q = p->right;
			while (q) {
				if (q->num == num) {
					p = cnfData::distroyClause(head, p);
					// std::cout << "distroy clause successfully" << std::endl;
					if (head->down == nullptr) {
						return satisfied;
					}
					break;
				}
				else if (q->num == -num) {
					q = cnfData::distroyWord(p, -num);
					// std::cout << "distroy word successfully: " << -num << std::endl;
					if (p->right == nullptr) {
						return unsatisfied;
					}
					continue;
				}
				q = q->next;
			}
			p = p->down;
		}
		return normal;
	}

	bool SAT::verifyResult(clauseNode* head, word* word_) {
		bool isCorrect = true;
		clauseNode* p = head->down;
		while (p && isCorrect) {
			isCorrect = false;
			wordNode* q = p->right;
			while (q) {
				if (q->num > 0 && word_[q->num - 1].truthValue == 1 ||
					q->num < 0 && word_[-q->num - 1].truthValue == 0) {
					isCorrect = true; 
					break;
				}
				q = q->next;
			}
			if (isCorrect == false) {
				return false;
			}
			p = p->down;
		}
		return isCorrect;
	}

	void SAT::displayResult(word* word_, int variableNum)
	{
		for (int i = 0; i < variableNum; i++) {
			std::cout << "num: " << i <<
				"\ntruthValue: " << word_[i].truthValue <<
				//"\nfrequency: " << word_[i].frequency <<
				//"\npolarity: " << word_[i].polarity <<
				std::endl;
		}
	}

	int SAT::isSameResult(word* solution1, word* solution2, int variableNum) {
		for (int i = 0; i < variableNum; i++) {
			if (solution1[i].truthValue != solution2[i].truthValue) {
				return i;
			}
		}
		return -1;
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="head"></param>
	/// <param name="word_"></param>
	/// <returns></returns>
	status SAT::CDCL(clauseNode* head, word*& word_) {
		int level = 0;
		// ������û���Ѿ����ڵĳ�ͻ
		status status_ = unitPropagation(head, word_, level);
		if (status_ == unsatisfied) {
			return normal;
		}
		while (assignedNum != variableNum) {
			// ���ѡһ�����ֲ£�����Ϊnullptr
			int var = pickVariable(head, word_);
			level++;
			applyAssignment(word_, var, level, nullptr);
			// ����
			while (true) {
				status_ = unitPropagation(head, word_, level);
				// ���ֳ�ͻ
				if (status_ == unsatisfied) {
					// ��ͻ�����ڵ�һ��
					if (level == 0) {
						return normal;
					}
					// �ҵ����緢����ͻ�Ĳ�
					level = clauseLearningAndBacktracking(head, word_, level);
				}
				else {
					break;
				}
			}
		}
		return success;
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="head"></param>
	/// <param name="word_"></param>
	/// <param name="decisionLevel"></param>
	/// <returns></returns>
	status SAT::unitPropagation(clauseNode*& head, word* word_, int decisionLevel) {
		bool isGet = false;
		int falseNum = 0;
		int unsetNum = 0;
		int lastUnset = 0;
		int var = -1;
		bool isSatisfied = false;
		do {
			isGet = false;
			clauseNode* p = head->down;
			while (p && !isGet) {
				falseNum = 0;
				unsetNum = 0;
				isSatisfied = false;
				wordNode* q = p->right;
				while (q) {
					var = abs(q->num) - 1;
					// ͳ��δ��ֵ�����ָ���
					if (word_[var].truthValue == -1) {
						unsetNum++;
						lastUnset = q->num;
					}
					// ��ֵ֮��ֵΪ��
					else if (word_[var].truthValue == 0 && q->num > 0 ||
						word_[var].truthValue == 1 && q->num < 0) {
						falseNum++;
					}
					else {
						isSatisfied = true;
						break;
					}
					q = q->next;
				}
				if (isSatisfied) {
					p = p->down;
					continue;
				}
				// ֻ��һ��δ��ֵ���ҵ����Ӿ䣬��¼�Ӿ�p
				if (unsetNum == 1) {
					applyAssignment(word_, lastUnset, decisionLevel, p);
					isGet = true;
					break;
				}
				// �������ֶ�Ϊ�٣����Ӿ䲻����
				else if (falseNum == p->wordNum) {
					antecedent = p;
					return unsatisfied;
				}
				p = p->down;
			}
		} while (isGet);
		antecedent = nullptr;
		return normal;
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="word_"></param>
	/// <param name="num"> q->num </param>
	/// <param name="level"></param>
	/// <param name="antecedent"></param>
	void SAT::applyAssignment(word* word_, int num, int level, clauseNode* antecedent) {
		int var = abs(num) - 1;
		word_[var].truthValue = num > 0 ? 1 : 0;
		word_[var].decesionLevel = level;
		word_[var].antecedent = antecedent;
		word_[var].frequency = -1;
		assignedNum++;
		return;
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="word_"></param>
	/// <param name="num"></param>
	void SAT::resetAssignment(word* word_, int num) {
		word_[num].truthValue = -1;
		word_[num].frequency = word_[num].frequency_bak;
		word_[num].decesionLevel = -1;
		word_[num].antecedent = nullptr;
		assignedNum--;
		return;
	}

	/// <summary>
	/// ���ҵ���ͻ�־䣬�ҵ����ݲ�����ȣ�Ȼ��reset�����ݲ������в�����֡�
	/// </summary>
	/// <param name="head"></param>
	/// <param name="word_"></param>
	/// <param name="level"></param>
	/// <returns></returns>
	int SAT::clauseLearningAndBacktracking(clauseNode*& head, word* word_, int level) {
		clauseNode* clauseLearned = antecedent;
		int conflictLevel = level;		// ������ͻ�Ĳ�
		int cnt = 0;					// ��ͬһ���ͻ������
		int resolver = 0;
		while (true) {
			cnt = 0;
			// ������Ҫѧϰ���־䣬�������д��ڳ�ͻ������ּ���
			wordNode* p = clauseLearned->right;
			while (p) {
				int var = abs(p->num) - 1;
				if (word_[var].decesionLevel == conflictLevel) {
					cnt++;
					// ͬʱ��¼���Ȳ�Ϊnullptr������
					if (word_[var].antecedent != nullptr) {
						resolver = var;
					}
				}
				p = p->next;
			}
			// ����ò�ֻ��һ�����ֳ�ͻ��
			if (cnt == 1) {
				break;
			}
			// ���ѧϰ�����Ӿ�
			clauseLearned = resolve(clauseLearned, word_, resolver);	//�ں���free��
			if (!clauseLearned) {
				return -1;
			}
		}
		// ��ѧϰ�����Ӿ���뵽�Ӿ伯�У�������word_
		clauseNode* p = head->down;
		head->down = clauseLearned;
		clauseLearned->down = p;
		wordNode* q = clauseLearned->right;
		while (q) {
			int var = abs(q->num) - 1;
			word_[var].polarity += q->num > 0 ? 1 : -1;
			if (word_[var].frequency != -1) {
				word_[var].frequency++;
			}
			word_[var].frequency_bak++;
			q = q->next;
		}
		head->wordNum++;
		// �ҵ����ݲ㣬Ҳ����ѧϰ�����Ӿ��в�����С���Ǹ�
		int backtrackedLevel = 0;
		q = clauseLearned->right;
		while (q) {
			int level = word_[abs(q->num) - 1].decesionLevel;
			if (level != conflictLevel && level > backtrackedLevel) {
				backtrackedLevel = level;
			}
			q = q->next;
		}
		// ��ԭ���ݲ��������и�ֵ
		p = head->down;
		for (int i = 0; i < variableNum; i++) {
			if (word_[i].decesionLevel > backtrackedLevel) {
				resetAssignment(word_, i);
			}
		}
		// free clauseLearned

		return backtrackedLevel;
	}

	/// <summary>
	/// �� ������ͻ�ľ��� �� BCP������ֵ�ľ��� ��ѧϰ
	/// </summary>
	/// <param name="clause"> ������ͻ���Ӿ� </param>
	/// <param name="word_"> </param>
	/// <param name="resolver"> �Ӿ������¸�ֵ�����֣�var��ʽ </param>
	/// <returns></returns>
	clauseNode* SAT::resolve(clauseNode* clause, word* word_, int resolver) {
		// �ҵ�resolver���������ڵ��Ӿ�
		clauseNode* antecedent = word_[resolver].antecedent;
		clauseNode* ret = (clauseNode*)malloc(sizeof(clauseNode));		// TODO free ret
		if (!ret) return nullptr;
		ret->right = (wordNode*)malloc(sizeof(wordNode));
		ret->wordNum = 0;
		ret->down = nullptr;
		// ȥ����clause��antecedent�м��resolver
		wordNode* p = antecedent->right;
		wordNode* q = ret->right;
		if (!q) return nullptr;
		q->num = 0;
		if (q && p->num - 1 != resolver && -p->num-1 != resolver) {	
			q->num = p->num;
			ret->wordNum++;
		}
		while (p) {
			if (q && p->num - 1 != resolver && -p->num - 1 != resolver) {
				if (q->num != 0) {
					q->next = (wordNode*)malloc(sizeof(wordNode));
					q = q->next;
				}
				if (!q) return nullptr;
				q->num = p->num;
				ret->wordNum++;
			}
			p = p->next;
		}
		p = clause->right;
		while (p) {
			if (q && p->num - 1 != resolver && -p->num - 1 != resolver) {
				q->next = (wordNode*)malloc(sizeof(wordNode));
				q = q->next;
				if (!q) return nullptr;
				q->num = p->num;
				ret->wordNum++;
			}
			p = p->next;
		}
		if (!q) return nullptr;
		q->next = nullptr;
		// retȥ��
		wordNode* prev = ret->right;
		std::unordered_map<int, int> umap;
		q = ret->right;
		while (q) {
			if (umap.find(q->num) == umap.end()) {
				umap[q->num] = 1;
			}
			else {
				ret->wordNum--;
				prev->next = q->next;
				wordNode* tmp = q;
				q = q->next;
				free(tmp);
				continue;
			}
			prev = q;
			q = q->next;
		}
		return ret;
	}

	/// <summary>
	/// ������һ����Ԫ
	/// </summary>
	/// <param name="head">  </param>
	/// <param name="word_">  </param>
	/// <returns> q->num </returns>
	int SAT::pickVariable(clauseNode* head, word* word_) {
		clauseNode* p = head->down;
		while (p) {
			wordNode* q = p->right;
			while (q) {
				int num = abs(q->num) - 1;
				if (word_[num].frequency > 0) {	/*TODO �ҵ�Ƶ����ߵ����� ע��Ҫ-1*/
					return q->num;
				}
				q = q->next;
			}
			p = p->down;
		}
		return 0;
	}
}