#include "Greedy.h"

Greedy::Greedy(WeightMatrix& wt):
	_input(wt),
	_result(wt.nodes(), W_UNALTERED)
{
	randPermutation(_perm, _input.nodes());
}

Greedy::~Greedy()
{
}

void Greedy::setIdentityPerm()
{
	int n = _perm.size();
	for(int ii = 0; ii < n; ii++)
	{
		_perm[ii] = ii;
	}
}	

WeightMatrix& Greedy::solve()
{
	int n = _perm.size();
	for(int ii = 0; ii < n; ii++)
	{
		markNode(_perm[ii], ii);
	}

	return _result;
}

void Greedy::assoc(int node, int other)
{
	int n = _input.nodes();
	for(int ii = 0; ii < n; ++ii)
	{
		if(_result(other, ii) == 1)
		{
			_result(node, ii) = 1;
		}
	}
}

Soon::Soon(WeightMatrix& wt):
	Greedy(wt)
{
}

void Soon::markNode(int node, int permIdx)
{
//	cerr<<"marking node "<<node<<"\n";

	for(int prev = permIdx - 1; prev >= 0; --prev)
	{
		int prevNode = _perm[prev];

// 		cerr<<"checking link with "<<prevNode<<
// 			" weight "<<_input.wdiff(node, prevNode)<<"\n";

		if(_input.wdiff(node, prevNode) > 0)
		{
// 			cerr<<"found assoc node "<<prevNode<<
// 				" link "<<_input.wdiff(node, prevNode)<<"\n";

			assoc(node, prevNode);
			break;
		}
	}
}

BestLink::BestLink(WeightMatrix& wt):
	Greedy(wt)
{
}

void BestLink::markNode(int node, int permIdx)
{
//	cerr<<"marking node "<<node<<"\n";

	int best = -1;
	float bestScore = 0;
	for(int prev = permIdx - 1; prev >= 0; --prev)
	{
		int prevNode = _perm[prev];
		if(_input.wdiff(node, prevNode) > bestScore)
		{
			best = prevNode;
			bestScore = _input.wdiff(node, prevNode);
		}
	}

// 	cerr<<"best link was "<<best<<
// 		" weight "<<bestScore<<"\n";

	if(best != -1)
	{
		assoc(node, best);
	}
}

VotedLink::VotedLink(WeightMatrix& wt):
	Greedy(wt),
	_nextCluster(1)
{
	_clusters.resize(_input.nodes());
}

WeightMatrix& VotedLink::solve()
{
	Greedy::solve();

	int n = _clusters.size();
	for(int ii = 0; ii < n; ++ii)
	{
		for(int jj = ii + 1; jj < n; ++jj)
		{
			if(_clusters[ii] == _clusters[jj])
			{
				_result(ii, jj) = 1;
			}
		}
	}

	return _result;
}

void VotedLink::markNode(int node, int permIdx)
{
	//there are _nextCluster - 1 clusters; the 0 space of the array is empty
	float votes[_nextCluster];
	for(int cluster = 1; cluster < _nextCluster; ++cluster)
	{
		votes[cluster] = 0;
	}

//	cerr<<"marking node "<<node<<"\n";

	for(int prev = permIdx - 1; prev >= 0; --prev)
	{
		int prevNode = _perm[prev];
		float vote = _input.wdiff(node, prevNode);

// 		cerr<<"vote for "<<prevNode<<" in cluster "<<_clusters[prevNode]<<
// 			" is "<<vote<<"\n";

		votes[_clusters[prevNode]] += vote;
	}

	int best = -1;
	float bestScore = 0;
	for(int cluster = 1; cluster < _nextCluster; ++cluster)
	{
//		cerr<<cluster<<"\t"<<votes[cluster]<<"\n";

		if(votes[cluster] > bestScore)
		{
			best = cluster;
			bestScore = votes[cluster];
		}
	}

	if(best != -1)
	{
		_clusters[node] = best;
	}
	else
	{
		_clusters[node] = _nextCluster++;
	}

//	cerr<<"chose "<<_clusters[node]<<"\n";
}

Pivot::Pivot(WeightMatrix& wt):
	Greedy(wt),
	_taken(wt.nodes())
{
}

void Pivot::markNode(int node, int permIdx)
{
//	cerr<<"Pivoting on "<<node<<"\n";

	if(_taken[node])
	{
//		cerr<<"Already clustered.\n";
		return;
	}

	int n = _perm.size();
	//can start at permIdx + 1, since 0..permIdx are taken
	for(int ii = permIdx + 1; ii < n; ++ii)
	{
		int nextNode = _perm[ii];

		if(_taken[nextNode])
		{
//			cerr<<"Skipping "<<nextNode<<"\n";
			continue;
		}

		if(_input.wdiff(node, nextNode) > 0)
		{
// 			cerr<<"Linking to "<<nextNode<<" weight "<<
// 				_input.wdiff(node, nextNode)<<"\n";

			assoc(nextNode, node);

			_taken[nextNode] = 1;
		}
	}
}
