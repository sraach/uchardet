require "test/unit"
require 'pp'

$:.unshift File.dirname(__FILE__) + "/../ext/uchardet"
require "uchardet.so"

class TestUchardetExtn < Test::Unit::TestCase
  
  def test_init
    assert_not_nil(ICU::UCharsetDetector)
    
    assert_nothing_raised do
      detector = ICU::UCharsetDetector.new
      assert_not_nil(detector)
    
      detector = ICU::UCharsetDetector.new nil
      assert_not_nil(detector)
    
      detector = ICU::UCharsetDetector.new 'some text'
      assert_not_nil(detector)
    end
    
    assert_raise(TypeError) do
      detector = ICU::UCharsetDetector.new 0
    end
    
    assert_raise(TypeError) do
      detector = ICU::UCharsetDetector.new Time.now
    end
  end
  
  def test_detect
    detector = ICU::UCharsetDetector.new
    assert_not_nil(detector)
    p detector.detect 'жабы отличаются отсутствием зубов в верхней челюсти, вполне развитым слуховым аппаратом, сильно развитыми околоушными кожными железами и треугольными'
  end
  
  def test_detect_all
    detector = ICU::UCharsetDetector.new
    assert_not_nil(detector)
    p detector.detect_all 'as if millions of voices suddenly cried out in terror and were suddenly silenced'
  end
  
  def test_detectable_charsets
    detector = ICU::UCharsetDetector.new
    assert_not_nil(detector)
    p detector.detectable_charsets
  end
  
  def test_input_filtered
    detector = ICU::UCharsetDetector.new
    assert_not_nil(detector)
    p detector.input_filtered?
    detector.input_filtered = true
    p detector.input_filtered?
    detector.input_filtered = ''
    p detector.input_filtered?
    detector.input_filtered = nil
    p detector.input_filtered?
  end
end