/*
 * Stack.cpp
 *
 *  Created on: 24 Oct 2015
 *      Author: hieu
 */
#include <algorithm>
#include <boost/foreach.hpp>
#include "Stack.h"
#include "Hypothesis.h"
#include "../Scores.h"

using namespace std;

Stack::Stack() {
	// TODO Auto-generated constructor stub

}

Stack::~Stack() {
	// TODO Auto-generated destructor stub
}

StackAdd Stack::Add(const Hypothesis *hypo)
{
  std::pair<iterator, bool> addRet = m_hypos.insert(hypo);
  if (addRet.second) {
    // equiv hypo doesn't exists
	return StackAdd(true, NULL);
  }
  else {
	  const Hypothesis *hypoExisting = *addRet.first;
	  if (hypo->GetScores().GetTotalScore() > hypoExisting->GetScores().GetTotalScore()) {
		  // incoming hypo is better than the one we have
		  m_hypos.erase(addRet.first);

		  // re-add. It better go in
		  std::pair<iterator, bool> addRet = m_hypos.insert(hypo);
		  assert(addRet.second);

		  return StackAdd(true, const_cast<Hypothesis*>(hypoExisting));
		  /*
		  const_cast<Hypothesis*>(hypo)->Swap(*const_cast<Hypothesis*>(hypoExisting));
		  return StackAdd(true, const_cast<Hypothesis*>(hypo));
		  */
	  }
	  else {
		  // already storing the best hypo. discard incoming hypo
		  return StackAdd(false, const_cast<Hypothesis*>(hypo));
	  }
  }
}

std::vector<const Hypothesis*> Stack::GetBestHyposAndPrune(size_t num, ObjectPoolContiguous<Hypothesis*> &recycler) const
{
  std::vector<const Hypothesis*> ret = GetBestHypos(num);
  if (num && ret.size() > num) {
	  for (size_t i = num; i < ret.size(); ++i) {
		  Hypothesis *hypo = const_cast<Hypothesis*>(ret[i]);
		  recycler.push(hypo);
	  }
	  ret.resize(num);
  }
  return ret;
}

std::vector<const Hypothesis*> Stack::GetBestHypos(size_t num) const
{
  std::vector<const Hypothesis*> ret(m_hypos.begin(), m_hypos.end());

  std::vector<const Hypothesis*>::iterator iterMiddle;
  iterMiddle = (num == 0 || ret.size() < num)
			   ? ret.end()
			   : ret.begin()+num;

  std::partial_sort(ret.begin(), iterMiddle, ret.end(),
		  HypothesisFutureScoreOrderer());

  return ret;
}

void Stack::Prune(Recycler<Hypothesis*> &recycler, size_t size, size_t maxSize)
{
  assert(size <= maxSize);
  if (GetSize() < maxSize) {
	return;
  }

  std::vector<const Hypothesis*> hypos = GetBestHypos(size);

  std::vector<const Hypothesis*>::iterator iter, iterNext;
	for (iter = hypos.begin() + size; iter != hypos.end(); ++iter) {
		Hypothesis *hypo = const_cast<Hypothesis*>(*iter);

		m_hypos.erase(hypo);
		recycler.push(hypo);

	}
}

