#include <iostream>
#include <vector>


void gen(int N, std::vector<std::vector<int>>& a) {
	int chance_mod;
	for (int q = 1; q <= (N / 2); q++) {
		for (int i = q; i < (N - q); i++) {
			for (int l = q; l < (N - q); l++) {
				if ((i == q || i == (N - 1) - q) || (l == q || l == (N - 1) - q)) {
					chance_mod = (q * 5);

					int neight_mod = 0;
					for (int sq = 0; sq < 4; sq++) {
						if (a[i++][l] == 1) neight_mod++; if (a[i--][l] == 1) neight_mod++; if (a[i][l++] == 1) neight_mod++; if (a[i][l--] == 1) neight_mod++;
					}
					neight_mod *= 15;

					int chance = 30 + chance_mod + neight_mod;

					int lot = rand() % 101;
					if (lot <= chance) {
						a[i][l] = 1;
					}
				}
			}
		}
	}

}