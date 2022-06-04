#pragma once

// blocklist是一组不重叠的block的有序集合
// {
//		[begin,end),
//		[begin,end),
//		[begin,end),
//		...
// }

class blocklist
{
public:
	blocklist()	{	}
	~blocklist()	{	}

public:
	typedef pair<uint64_t, uint64_t>	block_t;
	typedef vector<block_t>				blocks_t;
public:
	// 添加[begin,end)...自动处理重叠
	void insert(const uint64_t begin, const uint64_t end);
	void insert(const vector<block_t>& blockvector);
	void insert(const blocklist& blocklist);

	// 删除[begin,end)...
	void erase(const uint64_t begin, const uint64_t end);
	void erase(const vector<block_t>& blockvector);
	void erase(const blocklist& blocklist);

	// 取[begin,end)范围内的blocklist子集
	void subset(const uint64_t begin, const uint64_t end, vector<block_t>& blockvector) const;
	void subset(const uint64_t begin, const uint64_t end, blocklist& blocklist) const;

	// 遍历所有block计算总长度（TODO: 如果常常调用，需要进一步优化）
	uint64_t size(void) const;

	// 清空/是否为空
	void clear(void);
	bool empty(void) const;
	
	bool in(const uint64_t pos,block_t& range);

	bool intersection(const block_t& range);
	bool intersection(const uint64_t begin, const uint64_t end)
	{
		block_t range;
		range.first = begin;
		range.second = end;
		return intersection(range);
	}

public:
	blocks_t				m_blocklist;

	////////////////////////////////////////////////////////////////////////
	// 以下是测试代码
public:
	void dump(string& dump) const;
};
