require "test/unit"

$:.unshift File.dirname(__FILE__) + "/../ext/icu"
require "icu.so"

class TestIcuExtn < Test::Unit::TestCase
  def test_truth
    assert true
  end
end