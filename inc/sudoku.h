#pragma once

#include<string>
#include<fstream>
#include<random>

#include"dataLoader.h"
#include"solve.h"

#define ROW_CASE 1
#define COL_CASE 0

namespace dzh {
	// col: 1 - dim
	// row: 1 - dim

	/// <summary>
	/// ����˼�룺
	/// 1. Լ��һ������
	/// 2. Լ�������������
	/// 3. Լ������Tseytin�任
	/// 4. ��ȷ���Ŀ���Ϊ���Ӿ�����
	/// </summary>
	class Sudoku {
	public:
		/// <summary>
		/// �������еĿյ�Լ��
		/// </summary>
		/// <param name="path"></param>
		/// <param name="dim"></param>
		static void generateConstraint(std::string path, int dim);
		static void randomGuess(int dim, int* filled, int& size);
		static void combination(int n, int m, int& cnt);
		static int isSameResult(word* solution1, word* solution2, int variableNum);
	private:
		static void combination(int* src, int* dst, std::ofstream& fileWriter, int n, int m, int size);
		static void constraint3(int* src, int* dst, std::ofstream& fileWriter, int n, int m, int dim, int& start);
		static void Tseytin(std::ofstream& fileWriter, int dim, int i, int j, int start, int flag);
	};
}