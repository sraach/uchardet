$:.unshift(File.dirname(__FILE__)) unless
  $:.include?(File.dirname(__FILE__)) || $:.include?(File.expand_path(File.dirname(__FILE__)))

module Uchardet
  VERSION = '0.0.1'
end

begin
  require 'ext/uchardet/uchardet.so'
rescue LoadError
  # uh-oh
end

module ICU
  class UCharsetDetector
    def self.detect(*args)
      self.new.detect(*args)
    end
    
    def self.detect_all(*args)
      self.new.detect_all(*args)
    end
    
    def self.detectable_charsets
      self.new.detectable_charsets
    end
  end
end
