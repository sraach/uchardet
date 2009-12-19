$:.unshift(File.dirname(__FILE__)) unless
  $:.include?(File.dirname(__FILE__)) || $:.include?(File.expand_path(File.dirname(__FILE__)))

module Uchardet
  VERSION = '0.1.2'
end

begin
  require 'uchardet.so'
rescue LoadError
  # uh-oh
end

module ICU  # :main: README
  class UCharsetDetector  # :main: README
    ##
    # Shortcut for ICU::UCharsetDetector#detect
    #
    def self.detect(*args)
      self.new.detect(*args)
    end
    
    ##
    # Shortcut for ICU::UCharsetDetector#detect_all
    #
    def self.detect_all(*args)
      self.new.detect_all(*args)
    end
    
    ##
    # Shortcut for ICU::UCharsetDetector#detectable_charsets
    #
    def self.detectable_charsets
      self.new.detectable_charsets
    end
  end
end
