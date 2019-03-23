/**
   @author Alejandro Mujica
*/

void add_covered_position_around(const Board & board, size_t i, size_t j,
                                 list<Position> & l)
{
  if (i >= board.rows() or j >= board.cols())
    return;

  if (board.get_status(i, j) != StatusValue::COVERED)
    return;
  
  l.push_back(Position(i, j));
}

list<Position> get_covered_positions_around(const Board & board,
					    const Position & p)
{
  list<Position> l;
  
  add_covered_position_around(board, p.first - 1, p.second - 1, l);
  add_covered_position_around(board, p.first - 1, p.second + 0, l);
  add_covered_position_around(board, p.first - 1, p.second + 1, l);
  add_covered_position_around(board, p.first + 0, p.second - 1, l);
  add_covered_position_around(board, p.first + 0, p.second + 1, l);
  add_covered_position_around(board, p.first + 1, p.second - 1, l);
  add_covered_position_around(board, p.first + 1, p.second + 0, l);
  add_covered_position_around(board, p.first + 1, p.second + 1, l);
  
  return l;
}

void add_not_uncovered_position_around(const Board & board, size_t i, size_t j,
				       list<Position> & l)
{
  if (i >= board.rows() or j >= board.cols())
    return;
  
  if (board.get_status(i, j) == StatusValue::UNCOVERED)
    return;
  
  l.push_back(Position(i, j));
}

list<Position> get_not_uncovered_positions_around(const Board & board,
						  const Position & p)
{
  list<Position> l;
  
  add_not_uncovered_position_around(board, p.first - 1, p.second - 1, l);
  add_not_uncovered_position_around(board, p.first - 1, p.second + 0, l);
  add_not_uncovered_position_around(board, p.first - 1, p.second + 1, l);
  add_not_uncovered_position_around(board, p.first + 0, p.second - 1, l);
  add_not_uncovered_position_around(board, p.first + 0, p.second + 1, l);
  add_not_uncovered_position_around(board, p.first + 1, p.second - 1, l);
  add_not_uncovered_position_around(board, p.first + 1, p.second + 0, l);
  add_not_uncovered_position_around(board, p.first + 1, p.second + 1, l);
  
  return l;
}

bool is_util(const Board & board, size_t i, size_t j)
{
  if (board.get_status(i, j) != StatusValue::UNCOVERED)
    return false;
  
  if (board.get_num_mines_around(i, j) == 0)
    return false;
  
  list<Position> valid_positions =
    get_covered_positions_around(board, Position(i, j));
  
  for (Position & p : valid_positions)
    if (board.get_status(p.first, p.second) == StatusValue::COVERED)
      return true;
  
  return false;
}

list<Position> get_util_uncovered(const Board & board)
{
  list<Position> ret;

  for (size_t i = 0; i < board.rows(); ++i)
    for (size_t j = 0; j < board.cols(); ++j)
      if (is_util(board, i, j))
	ret.push_back(Position(i, j));

  return ret;
}

pair<bool, Position> get_first(const Board & board, size_t diff)
{
  Position p;

  for (p.first = 0; p.first < board.rows(); ++p.first)
    for (p.second = 0; p.second < board.cols(); ++p.second)
      {
        if (board.get_status(p.first, p.second) != StatusValue::UNCOVERED)
	  continue;
	
        list<Position> l = get_covered_positions_around(board, p);

	size_t sz = l.size();
	size_t num_mines = board.get_num_mines_around(p.first, p.second);

	if (sz < num_mines)
	  continue;

	if (sz - num_mines == diff)
	  return make_pair(true, p);
      }

  return make_pair(false, p);
}

list<Position> get_covered_positions(const Board & board)
{
  list<Position> pos_set;
  
  for (size_t i = 0; i < board.rows(); ++i)
    for(size_t j = 0; j < board.cols(); ++j)
      if (board.get_status(i, j) == StatusValue::COVERED)
	pos_set.push_back(make_pair(i, j));

  return pos_set;
}

pair<bool, Position> select_random_covered(const Board & board, mt19937 & rng)
{
  list<Position> pos_set = get_covered_positions(board);
  
  if (pos_set.empty())
    return make_pair(false, Position());

  uniform_int_distribution<int> unif(0, pos_set.size() - 1);
  auto p = unif(rng);

  auto it = pos_set.begin();
  
  while (p-- > 0)    
    ++it;

  return make_pair(true, *it);
}

pair<bool, Position> get_a_position(const Board & board, mt19937 & rng)
{
  if (board.get_status(0, 0) == StatusValue::COVERED)
    return make_pair(true, make_pair(0, 0));
  
  if (board.get_status(0, board.cols() - 1) == StatusValue::COVERED)
    return make_pair(true, make_pair(0, board.cols() - 1));
		     
  if (board.get_status(board.rows() - 1, 0) == StatusValue::COVERED)
    return make_pair(true, make_pair(board.rows() - 1, 0));
  
  if (board.get_status(board.rows() - 1, board.cols() - 1) ==
          StatusValue::COVERED)
    return make_pair(true, make_pair(board.rows() - 1, board.cols() - 1));
  
  for (auto i = 4; i > 1; --i)
    {
      pair<bool, Position> fp = get_first(board, i);

      if (not fp.first)
	continue;

      list<Position> s = get_covered_positions_around(board, fp.second);

      assert(s.size() > 0);

      uniform_int_distribution<int> unif(0, s.size() - 1);
      auto p = unif(rng);
      
      auto it = s.begin();
      
      while (p-- > 0)
	++it;
      
      return make_pair(true, *it);
    }
  
  return select_random_covered(board, rng);
}

list<Position> get_for_flags(const Board & board)
{
  list<Position> ret;
  
  list<Position> util_uncovered = get_util_uncovered(board);
  
  for (const Position & p : util_uncovered)
    {
      list<Position> l = get_not_uncovered_positions_around(board, p);
	
      const auto & num_covered = l.size();
	
      if (num_covered >
	  board.get_num_mines_around(p.first, p.second))
	continue;
	
      for (const Position & q : l)
	{
          if (board.get_status(q.first, q.second) == StatusValue::FLAG)
	    continue;
	  
	  if (find(ret.begin(), ret.end(), q) == ret.end())
	    ret.push_back(q);
	}
    }
    
  return ret;
}
  
list<Position> get_for_discover(const Board & board)
{
  list<Position> ret;

  list<Position> util_uncovered = get_util_uncovered(board);

  for (const Position & p : util_uncovered)
    {
      list<Position> l = get_not_uncovered_positions_around(board, p);
	
      int num_flags = 0;

      for (const Position & q : l)
	{
          if (board.get_status(get<0>(q), get<1>(q)) == StatusValue::FLAG)
	    ++num_flags;
	}
	
      if (num_flags < board.get_num_mines_around(p.first, p.second))
	continue;

      for (const Position & q : l)
	{
          if (board.get_status(q.first, q.second) == StatusValue::FLAG)
	    continue;
	  
	  if (find(ret.begin(), ret.end(), q) == ret.end())
	    ret.push_back(q);
	}
    }
  return ret;
}
  
void solve(Board & board, mt19937 & rng)
{
  while (not board.are_uncovered_all())
    {
      auto p = get_a_position(board, rng);

      if (not p.first)
	return;
      
      board.discover(p.second.first, p.second.second);

      if (board.is_uncovered_mine(p.second.first, p.second.second))
	return;
	
      bool has_discovered = true;
      
      while (has_discovered)
	{
	  list<Position> f = get_for_flags(board);

	  for (auto p : f)
	    {
	      assert(get<0>(board.hint(p.first, p.second)));
	      board.flag(p.first, p.second);
	    }

	  list<Position> d = get_for_discover(board);
	  
          has_discovered = d.size() > 0;
	  
	  for (auto p : d)
	    {
	      board.discover(p.first, p.second);
              assert(not board.is_uncovered_mine(p.first, p.second));
	    }
	}
    }
}
