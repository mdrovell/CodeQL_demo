/**
 * @name Array access with literal index out of bounds
 * @description Direct array access with a constant index that exceeds array size
 * @kind problem
 * @problem.severity error
 * @id cpp/array-literal-index-out-of-bounds
 */

import cpp

from ArrayExpr access, ArrayType arrayType, int index, int size
where
  arrayType = access.getArrayBase().getType().getUnspecifiedType() and
  arrayType.getArraySize() = size and
  access.getArrayOffset().getValue().toInt() = index and
  index >= size
select access, "Array access at index " + index + " exceeds array size of " + size